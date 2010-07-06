/*
  ==============================================================================

   This file is part of the OSCIT library (http://rubyk.org/liboscit)
   Copyright (c) 2007-2010 by Gaspard Bucher - Buma (http://teti.ch).

  ------------------------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

  ==============================================================================
*/

#include "test_helper.h"
#include "oscit/root.h"
#include "oscit/root_proxy.h"
#include "mock/command_logger.h"
#include "mock/proxy_factory_logger.h"
#include "mock/object_proxy_logger.h"
#include "mock/observer_logger.h"

class ObjectProxyTest : public TestHelper
{
public:

  void should_set_root_proxy_when_adopted( void ) {
    RootProxy proxy(Location("osc", "funky synth"));
    Logger logger;
    assert_equal("", logger.str());
    ObjectProxyLogger *object = new ObjectProxyLogger("foobar", Attribute::no_io("info"), &logger);
    object->set_stream(&logger);
    proxy.adopt(object);
    assert_equal("[foobar: adopted RootProxy]", logger.str());
    assert_equal(&proxy, object->root_proxy());
  }

  void should_receive_value_changed_from_rootproxy( void ) {
    RootProxy proxy(Location("osc", "funky synth"));
    Logger logger;
    ObserverLogger observer("value_changed", &logger);
    ObjectProxy *seven = proxy.adopt(new ObjectProxy("seven", Attribute::range_io(0.0, 2000.0, "the sky is blue")));
    seven->on_value_change().connect(&observer, &ObserverLogger::event);
    logger.str("");
    proxy.handle_reply(std::string("/seven"), Value(300.0));
    assert_equal("[value_changed: 300]", logger.str());
  }

  void test_update_remote_on_set_value( void ) {
    Logger logger;
    CommandLogger cmd("osc", &logger);
    RootProxy *proxy = cmd.adopt_proxy(new RootProxy(Location("osc", "funky synth")));
    ObjectProxyLogger *obj = proxy->adopt(new ObjectProxyLogger("seven", Attribute::range_io(0.0, 2000.0, "the sky is blue"), &logger));
    logger.str("");
    obj->set_value(Value(45.0));
    assert_equal("[osc: send osc://\"funky synth\" /seven 45]", logger.str());
  }

  void test_latency( void ) {
    Logger logger;
    CommandLogger cmd("osc", &logger);
    RootProxy *proxy = cmd.adopt_proxy(new RootProxy(Location("osc", "funky synth")));
    ObjectProxyLogger *obj = proxy->adopt(new ObjectProxyLogger("seven", Attribute::range_io(0.0, 2000.0, "the sky is blue"), &logger));
    logger.str("");
    obj->set_value(Value(45.0));
    millisleep(12);
    obj->handle_value_change(Value(45.0));
    assert_true( 11 <= obj->latency() && obj->latency() <= 13 );
  }

  void test_sync_should_call_list_with_attributes( void ) {
    Logger logger;
    CommandLogger cmd("osc", &logger);
    RootProxy *proxy = cmd.adopt_proxy(new RootProxy(Location("osc", "funky synth")));
    ObjectProxyLogger *obj = proxy->adopt(new ObjectProxyLogger("seven", Attribute::range_io(0.0, 2000.0, "the sky is blue"), &logger));
    logger.str("");
    obj->sync_children();
    assert_equal("[osc: send osc://\"funky synth\" /.list_att \"/seven\"]", logger.str());
  }

  void test_new_object_without_type_should_try_to_find_type_and_initial_value( void ) {
    Logger logger;
    CommandLogger cmd("osc", &logger);
    RootProxy *proxy = cmd.adopt_proxy(new RootProxy(Location("osc", "funky synth")));
    logger.str("");
    proxy->adopt(new ObjectProxy("seven", gNilValue));
    assert_equal("[osc: send osc://\"funky synth\" /.type \"/seven\"][osc: send osc://\"funky synth\" /seven null]", logger.str());
  }

  void test_until_type_is_set_object_proxy_should_respond_false_to_is_connected( void ) {
    ObjectProxy o("name", gNilValue);
    assert_false( o.is_connected() );
    Value res = o.trigger(gNilValue);
    assert_true( res.is_nil() );
    o.set_attrs(Attribute::range_io(0.0, 200.0, "hop hop"));
    assert_true( o.is_connected() );
  }
};
