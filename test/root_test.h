/*
  ==============================================================================

   This file is part of the OSCIT library (http://rubyk.org/liboscit)
   Copyright (c) 2007-2009 by Gaspard Bucher - Buma (http://teti.ch).

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
#include "mock/dummy_object.h"
#include "mock/observer_logger.h"
#include "mock/command_logger.h"
#include "mock/object_logger.h"
#include "mock/command_logger.h"

class RootTest : public TestHelper
{
public:
  void test_create( void ) {
    Root r;
    assert_equal("", r.name());
    assert_equal("", r.url());
  }

  void test_url( void ) {
    Root r("funky");
    assert_equal("funky", r.name());
    assert_equal("", r.url());
    Object *lala = r.adopt(new Object("lala"));
    assert_equal("/lala", lala->url());
  }

  void test_get_object_at( void ) {
    Root root;
    Object * foo = root.adopt(new Object("foo"));
    Object * bar = foo->adopt(new Object("bar"));
    ObjectHandle res;

    assert_equal(""        , root.url() );
    assert_equal("/foo"    , foo->url() );
    assert_equal("/foo/bar", bar->url() );

    assert_true(root.get_object_at("", &res));
    assert_equal(&root, res.ptr() );

    res = NULL; // clear handle: set handle not allowed in get_object_at.
    assert_true(root.get_object_at("/foo", &res));
    assert_equal(foo, res.ptr() );

    res = NULL;
    assert_true(root.get_object_at("/foo/bar", &res));
    assert_equal(bar, res.ptr() );


    res = NULL;
    assert_false( root.get_object_at("/super", &res));
    assert_false( root.get_object_at("/super/bar", &res));

    foo->set_name("super");

    res = NULL;
    assert_true(root.get_object_at("/super", &res));
    assert_equal(foo, res.ptr() );

    res = NULL;
    assert_true(root.get_object_at("/super/bar", &res));
    assert_equal(bar, res.ptr() );

    res = NULL;
    assert_false( root.get_object_at("/foo", &res));
    assert_false( root.get_object_at("/foo/bar", &res));
  }

  void test_get_object_at_same_name_as_sibling( void ) {
    Root root;
    DummyObject * a  = new DummyObject("a", 1);
    DummyObject * a2 = new DummyObject("a", 2);
    ObjectHandle res;

    assert_true( root.get_object_at("", &res) );
    assert_equal(&root, res.ptr() );

    res = NULL; // clear handle: set handle not allowed in get_object_at.
    assert_false( root.get_object_at("/a", &res) );

    root.adopt(a);

    res = NULL;
    assert_true( root.get_object_at("/a", &res) );
    assert_equal(a, res.ptr() );

    root.adopt(a2);
    res = NULL;
    assert_true( root.get_object_at("/a", &res) );
    assert_equal(a, res.ptr() );

    res = NULL;
    assert_true( root.get_object_at("/a-1", &res) );
    assert_equal(a2, res.ptr() );

    root.clear();
    res = NULL;
    assert_false(root.get_object_at("/a", &res) );
    assert_false(root.get_object_at("/a-1", &res) );
    assert_true( root.get_object_at("", &res) );
    assert_equal(&root, res.ptr() );
  }

  void test_call_without_arguments_should_return_current_value( void ) {
    Root root;
    root.adopt(new DummyObject("zorglub", 9.87));
    Value res = root.call("/zorglub");
    assert_true(res.is_real());
    assert_equal(9.87, res.r);
  }

  void test_call_with_nil_should_return_current_value( void ) {
    Root root;
    root.adopt(new DummyObject("zorglub", 9.87));
    Value res = root.call("/zorglub", gNilValue);
    assert_true(res.is_real());
    assert_equal(9.87, res.r);
  }

  void test_call_with_empty_should_return_current_value( void ) {
    Root root;
    root.adopt(new DummyObject("zorglub", 9.87));
    Value res = root.call("/zorglub", Value());
    assert_true(res.is_real());
    assert_equal(9.87, res.r);
  }

  void test_call_with_bad_arguments_should_return_bad_request_error( void ) {
    Root root;
    root.adopt(new DummyObject("zorglub", 9.87));
    Value param(TypeTag("ff"));
    Value res = root.call("/zorglub", param);
    assert_true(res.is_error());
    assert_equal(std::string("'/zorglub' (").append(DUMMY_OBJECT_INFO).append(")."), res.error_message());
    assert_equal(BAD_REQUEST_ERROR, res.error_code());
  }

  void test_call_bad_url_should_return_missing_error( void ) {
    Root root;
    Value res = root.call("/foo");

    assert_true(res.is_error());
    assert_equal("/foo", res.error_message());
    assert_equal(NOT_FOUND_ERROR, res.error_code());
  }

  void test_call_should_route_message_to_object( void ) {
    Root root;
    DummyObject *dummy = root.adopt(new DummyObject("dummy", 0.0));
    Value res = root.call("/dummy", Value(4.5));

    assert_false(res.is_error());
    assert_equal(4.5, dummy->real());
  }

  void test_find_or_build_object_at_should_call_build_child( void ) {
    Root root;
    root.adopt(new DummyObject("dummy", 0.0));
    Value error;
    ObjectHandle object;

    assert_false(root.find_or_build_object_at("whatever", &error, &object));
    assert_false(root.find_or_build_object_at("/whatever", &error, &object));
    assert_false(root.find_or_build_object_at("/dummy/foo", &error, &object));
    assert_true(error.is_error());
    assert_equal(NOT_FOUND_ERROR, error.error_code());
    assert_false(root.find_or_build_object_at("/dummy/error", &error, &object));
    assert_true(error.is_error());
    assert_equal("You should not try to build errors !", error.error_message());
    assert_equal(INTERNAL_SERVER_ERROR, error.error_code());

    assert_true(root.find_or_build_object_at("/dummy/special", &error, &object));
    assert_equal("/dummy/special", object->url());
  }

  void test_call_should_build_child( void ) {
    Root root;
    root.adopt(new DummyObject("builder", 0.0));
    Value res = root.call("/builder/AgeOf/Capitain"); // dummy builds 'AgeOf' and 'Capitain'
    assert_true(res.is_real());
    assert_equal(78.0, res.r);
  }

  void test_adopt_command_should_start_listeners( void ) {
    Root root;
    Logger logger;
    root.adopt_command(new CommandLogger("osc", &logger));
    assert_equal("[osc: listen][osc: .]", logger.str());
  }

  void test_adopt_command_with_false_should_not_start_listeners( void ) {
    Root root;
    Logger logger;
    root.adopt_command(new CommandLogger("osc", &logger), false);
    assert_equal("", logger.str());
  }

  void test_remote_object_at( void ) {
    Root root;
    Logger logger;
    root.adopt_command(new CommandLogger(&logger));
    DummyObject  *foo = root.adopt(new DummyObject("foo", 3));
    ObjectHandle res;
    Value error;
    assert_true(root.get_object_at(Url("/foo"), &error, &res));
    assert_equal((Object*)foo, res.ptr());
    assert_true(error.is_empty());
  }

  void test_object_at_bad_protocol( void ) {
    Root root;
    Value error;
    ObjectHandle object;
    assert_false(root.get_object_at(Url("some://example.com:80/foo"), &error, &object));
    assert_equal(BAD_REQUEST_ERROR, error.error_code());
    assert_equal("No command to handle \'some\' protocol.", error.error_message());
  }

  void test_object_at_bad_url( void ) {
    Root root;
    Value error;
    ObjectHandle object;
    assert_false(root.get_object_at(Url("some://example.com /foo"), &error, &object));
    assert_equal(BAD_REQUEST_ERROR, error.error_code());
    assert_equal("Could not parse url \'some://example.com /foo\'.", error.error_message());
  }

  void test_send_should_route_messages_depending_on_protocol( void ) {
    Root root;
    Logger logger;
    root.adopt_command(new CommandLogger("http", &logger),false);
    root.adopt_command(new CommandLogger("osc", &logger),false);
    logger.str("");
    root.send(Location("http", "this place"), "/foo/bar", gNilValue);
    assert_equal("[http: send http://\"this place\" /foo/bar null]", logger.str());
    logger.str("");
    root.send(Location("osc", "funky thing"), "/one/two", gNilValue);
    assert_equal("[osc: send osc://\"funky thing\" /one/two null]", logger.str());
  }

  void should_only_register_one_command_per_protocol( void ) {
    Root root;
    Logger logger;
    root.adopt_command(new CommandLogger("one", "osc", &logger),false);
    CommandLogger *two = root.adopt_command(new CommandLogger("two", "osc", &logger),false);
    assert_equal(NULL, (void*)two);
    assert_equal("", logger.str());
    logger.str("");
    root.send(Location("osc", "some place"), "/foo/bar", gNilValue);
    assert_equal("[one: send osc://\"some place\" /foo/bar null]", logger.str());
  }

  void test_named_root( void ) {
    Root root("gaia");
    ObjectHandle object;
    assert_true(root.get_object_at("", &object));
    assert_equal((Object*)&root, object.ptr());
  }

  void test_delete_command_should_unregister_it_from_observers( void ) {
    Root root;
    root.adopt(new DummyObject("foo", 4.5));
    Logger logger;
    CommandLogger *cmd = root.adopt_command(new CommandLogger("osc", &logger));
    delete cmd;
    logger.str("");
    root.call("/foo", Value(5.2));
  }

  void test_list_with_type_on_root( void ) {
    Root root;
    root.adopt(new DummyObject("mode", "rgb", SelectIO("rgb, yuv", "color mode", "This is a menu.")));
    root.adopt(new DummyObject("tint", 45.0, RangeIO(1, 127, "tint", "This is a slider from 1 to 127.")));
    Value res = root.list_with_type();
    // .error, .info, etc are ignored (current value -- type mismatch)
    assert_equal("[s[*s]][s[sss]][s[sss]][s[sss]][s[sss]][s[sss]][s[ssss]][s[fffss]]", res.type_tag());
    assert_equal(".error", res[0][0].str());
    assert_equal(".info", res[1][0].str());
  }

  void test_create_root_without_meta( void ) {
    Root root(false);
    Value res = root.list();
    assert_true(res.size() == 0);
  }

  void should_destroy_all_tree_on_delete( void ) {
    Root *root = new Root;
    Logger logger;
    root->adopt(new ObjectLogger("one", &logger));
    assert_equal("", logger.str());
    delete root;
    assert_equal("[one: destroyed]", logger.str());
  }

  void test_make_views_path( void ) {
    Root root(false);
    Value res = root.list();
    assert_equal("[]", res.to_json());
    ObjectHandle views;
    assert_true(root.get_views_path(&views));
    res = root.list();
    assert_equal("[\"views\"]", res.to_json());
    // make again should return same object
    ObjectHandle views2;
    assert_true(root.get_views_path(&views2));
    assert_equal(views.ptr(), views2.ptr());
    res = root.list();
    assert_equal("[\"views\"]", res.to_json());
  }

  void should_accept_registration_notifications( void ) {
    Root root(false);
    Logger logger;
    ObserverLogger observer("observer", &logger);
    root.adopt_callback_on_register(std::string("/foo/bar"),
      new ObserverLogger::OnRegistrationCallback(&observer, "/foo/bar")
    );
    assert_equal("", logger.str());
    Object *foo = root.adopt(new Object("foo"));
    assert_equal("", logger.str());
    foo->adopt(new Object("bar"));
    // trigger, and lock/unlock during removal 'produced callbacks'.
    assert_equal("[observer: on_registration_callback /foo/bar][observer: lock][observer: unlock]", logger.str());
  }

  void should_expose_files_as_views( void ) {
    Root root(false);
    Value error;
    assert_true(root.expose_views(fixture_path(FILE_TEST_LIST_FOLDER), &error));
    ObjectHandle views;
    assert_true(root.get_object_at(VIEWS_PATH, &views));
    Value list = views->list_with_type();
    assert_equal("[[\"file_a/\", [{\"x\":10, \"y\":10, \"width\":500, \"height\":270}, \"view\"]], [\"file_b/\", [{}, \"view\"]]]", list.to_json());
  }

  // remote objects and 'send' testing is done in command_test.h
};
