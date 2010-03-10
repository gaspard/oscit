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

class RootProxyTest : public TestHelper
{
public:
  void should_send_commands_to_mirror_remote_tree( void ) {
    Root root;
    Logger logger;
    ProxyFactoryLogger factory("factory", &logger);
    Location location("oscit", "my place");
    CommandLogger *cmd = root.adopt_command(new CommandLogger("oscit", &logger));
    assert_equal("[oscit: listen][oscit: .]", logger.str());
    logger.str("");
    cmd->adopt_proxy(factory.build_and_init_root_proxy(location));
    assert_equal("[factory: build_root_proxy oscit://\"my place\"][oscit: send oscit://\"my place\" /.register null][oscit: send oscit://\"my place\" /.list_with_type \"\"]", logger.str());
  }

  void test_route_reply_messages_to_object_proxies( void ) {
    RootProxy proxy(Location("osc", "funky synth"));
    Logger logger;
    proxy.adopt(new ObjectProxyLogger("seven", RangeIO(0.0, 2000.0, "the sky is blue"), &logger));
    logger.str("");
    proxy.handle_reply(std::string("/seven"), Value(300.0));
    assert_equal("[seven: value_changed 300]", logger.str());
  }

  void should_build_object_proxies_without_type( void ) {
    RootProxy proxy(Location("osc", "funky synth"));
    Logger logger;
    ProxyFactoryLogger factory("factory", &logger);
    proxy.set_proxy_factory(&factory);

    ObjectHandle object;
    assert_false(proxy.get_object_at("/one/two", &object));
    assert_false(proxy.get_object_at("/one", &object));

    Value error;
    ObjectHandle object2;
    assert_true(proxy.find_or_build_object_at(std::string("/one/two"), &error, &object2));
    assert_equal("[factory: build_object_proxy one null][factory: build_object_proxy two null]", logger.str());
    logger.str("");
    assert_false(object2.ptr() == NULL);

    assert_true(proxy.get_object_at("/one/two", &object));
    assert_equal(object2.ptr(), object.ptr());
    assert_equal(object->type(), gNilValue);

    object = NULL; // clear handle (handle already set not allowed in root code)
    assert_true(proxy.get_object_at("/one", &object));
    assert_equal(object->type(), gNilValue);
    assert_equal("", logger.str());
  }
};
