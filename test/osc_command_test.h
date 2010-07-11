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
#include "mock/object_logger.h"
#include "oscit/list_meta_method.h"
#include "oscit/list_with_attributes_meta_method.h"

#include "mock/dummy_object.h"
#include "mock/osc_command_logger.h"

class OscCommandTest : public TestHelper
{
 public:
  enum {
    RECEIVER_PORT = 7014,
    SENDER_PORT   = 7015
  };

  OscCommandTest() : remote_end_point_(Location::LOOPBACK, RECEIVER_PORT) {
    remote_.adopt_command(new OscCommandLogger(RECEIVER_PORT, "receiver", &reply_));

    sender_ = local_.adopt_command(new OscCommandLogger(SENDER_PORT, "sender", &reply_));
    // we need to register in order to get return values
    send("/.register", gNilValue);
  }

  void setUp() {
    remote_.Object::clear(); // empty root but keep commands
    logger_.str("");
  }

  // ================================================================= Empty (null) = GET
  void test_send_empty_should_receive_value( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", 1.25));

    send("/foo", Value());
    assert_equal(1.25, foo->real());
    assert_equal("[\"/foo\", 1.25]\n", reply());
  }

  void test_send_nil_should_receive_value( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", 1.25));

    send("/foo", gNilValue);
    assert_equal(1.25, foo->real());
    assert_equal("[\"/foo\", 1.25]\n", reply());
  }

  // ================================================================= True
  void test_send_receive_true( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "true", Oscit::io("Info", "true", "T")));

    send("/foo", JsonValue("true"));
    assert_equal("true", foo->value_.to_json());
    assert_equal("[\"/foo\", true]\n", reply());
  }

  // ================================================================= False
  void test_send_receive_false( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "false", Oscit::io("Info", "false", "F")));

    send("/foo", JsonValue("false"));
    assert_equal("false", foo->value_.to_json());
    assert_equal("[\"/foo\", false]\n", reply());
  }

  // ================================================================= Real
  void test_send_receive_real( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "1.0", Oscit::real_io("Info")));

    send("/foo", JsonValue("3.2"));
    assert_equal("3.2", foo->value_.to_json());
    assert_equal("[\"/foo\", 3.2]\n", reply());
  }

  // ================================================================= String
  void test_send_receive_string( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "\"hello\"", Oscit::string_io("Info")));

    send("/foo", JsonValue("\"goodbye\""));
    assert_equal("\"goodbye\"", foo->value_.to_json());
    assert_equal("[\"/foo\", \"goodbye\"]\n", reply());
  }

  // ================================================================= Error
  void test_send_receive_error( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "[400, \"bad\"]", Oscit::io("Info", "error", "fs")));
    // Cannot receive errors with 'E' for the moment: need to set signature to 'fs'.
    send("/foo", ErrorValue(NOT_FOUND_ERROR, "not ok"));
    assert_equal("[404, \"not ok\"]", foo->value_.to_json());
    assert_equal("[\"/foo\", [404, \"not ok\"]]\n", reply());
  }

  // ================================================================= Hash
  void test_send_receive_hash( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "{one:1, two:2}", Oscit::hash_io("Info")));

    send("/foo", JsonValue("{\"forty\":40, \"seven\":\"seven\", \"nested\":{\"inner\":[1, 2, 3]}}"));
    assert_equal("{\"forty\":40, \"seven\":\"seven\", \"nested\":{\"inner\":[1, 2, 3]}}", foo->value_.to_json());
    assert_equal("[\"/foo\", {\"forty\":40, \"seven\":\"seven\", \"nested\":{\"inner\":[1, 2, 3]}}]\n", reply());
  }

  // ================================================================= Matrix
  //Not supported yet
  //void test_send_receive_matrix( void ) {
  //  DummyObject * foo = remote_.adopt(new DummyObject("foo", " matrix ? ", Oscit::matrix_io("Info.")));
  //
  //  send("/foo", JsonValue("true"));
  //  assert_equal("", foo->value_.to_json());
  //  assert_equal("[\"/foo\", 3]\n", reply());
  //}

  // ================================================================= Midi
  void test_send_receive_midi( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "{\"=m\":[147, 60, 80, 400]}", Oscit::midi_io("Info")));

    send("/foo", JsonValue("{\"=m\":[147, 67, 100, 100]}"));
    assert_equal("{\"=m\":[147, 67, 100, 100]}", foo->value_.to_json());
    assert_equal("[\"/foo\", {\"=m\":[147, 67, 100, 100]}]\n", reply());
  }

  // ================================================================= Any
  void test_send_receive_any( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "null", Oscit::any_io("Info")));

    send("/foo", JsonValue("[{hello:3}, 4, 5]"));
    assert_equal("[{\"hello\":3}, 4, 5]", foo->value_.to_json());
    assert_equal("[\"/foo\", [{\"hello\":3}, 4, 5]]\n", reply());
  }

  // ================================================================= List
  void test_send_receive_list( void ) {
    DummyObject * foo = remote_.adopt(new DummyObject("foo", "null", Oscit::io("Info", "list", "ffs[ss[ff]]")));

    send("/foo", JsonValue("[1.5, -3.2, 'bar', ['a', 'b', [1, 2]]]"));
    assert_equal("[1.5, -3.2, \"bar\", [\"a\", \"b\", [1, 2]]]", foo->value_.to_json());
    assert_equal("[\"/foo\", [1.5, -3.2, \"bar\", [\"a\", \"b\", [1, 2]]]]\n", reply());
  }

  // ================================================================= /.list meta method

  void test_send_receive_list_meta_method( void ) {
    // remote_ objects are cleared before each run
    remote_.adopt(new ListMetaMethod(Url(LIST_PATH).name()));
    Object * tmp = remote_.adopt(new Object("monitor"));
    tmp->adopt(new DummyObject("mode", "rgb", Oscit::select_io("This is a menu.", "rgb, yuv")));
    tmp->adopt(new DummyObject("tint", 45.0, Oscit::range_io("This is a slider from 1 to 127.", 1, 127)));

    send(LIST_PATH, "/monitor");
    assert_equal("[\"/.list\", [\"/monitor\", [\"mode\", \"tint\"]]]\n", reply());
  }

  void test_send_receive_list_on_empty( void ) {
    // remote_ objects are cleared before each run
    remote_.adopt(new ListMetaMethod(Url(LIST_PATH).name()));
    remote_.adopt(new Object("monitor"));

    send(LIST_PATH, "/monitor");
    assert_equal("[\"/.list\", [\"/monitor\", []]]\n", reply());
  }

  void test_send_receive_list_with_attributes( void ) {
    // remote_ objects are cleared before each run
    remote_.adopt(new ListWithOscitsMetaMethod(Url(LIST_WITH_ATTRIBUTES_PATH).name()));
    Object * tmp = remote_.adopt(new Object("monitor"));
    tmp->adopt(new DummyObject("mode", "rgb", Oscit::select_io("This is a menu.", "rgb, yuv")));
    tmp->adopt(new DummyObject("tint", 45.0, Oscit::range_io("This is a slider from 1 to 127.", 1, 127)));

    send(LIST_WITH_ATTRIBUTES_PATH, "/monitor");
    assert_equal("[\"/.list_att\", [\"/monitor\", [\"mode\", {\"@info\":\"This is a menu.\", \"@type\":{\"name\":\"select\", \"signature\":\"s\", \"values\":\"rgb, yuv\"}}, \"tint\", {\"@info\":\"This is a slider from 1 to 127.\", \"@type\":{\"name\":\"range\", \"signature\":\"f\", \"min\":1, \"max\":127}}]]]\n", reply());
  }

 private:
  void send(const char *url, Real real) {
    send(url, Value(real));
  }

  void send(const char *url, const char *str) {
    send(url, Value(str));
  }

  void send(const char *url, const Value &val) {
    millisleep(10);
    reply_.str("");
    sender_->clear_replies();
    sender_->send(remote_end_point_, url, val);
    millisleep(10);
  }

  const std::string reply() {
    millisleep(10);
    return sender_->replies();
  }

  Logger logger_;
  Location remote_end_point_;
  Logger reply_;
  Root remote_;
  Root local_;
  OscCommandLogger *sender_;
};
