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
#include "oscit/values.h"

class StringValueTest : public TestHelper
{
public:
  void test_is_string( void ) {
    Value v("foo");

    assert_false(v.is_empty());
    assert_false(v.is_nil());
    assert_false(v.is_true());
    assert_false(v.is_false());
    assert_false(v.is_real());
    assert_true (v.is_string());
    assert_false(v.is_list());
    assert_false(v.is_error());
    assert_false(v.is_hash());
    assert_false(v.is_matrix());
    assert_false(v.is_midi());

    assert_equal("foo", v.str());

    assert_equal("s", v.type_tag());
    assert_equal(STRING_TYPE_TAG_ID, v.type_id());

    assert_equal(STRING_TYPE_TAG_ID, hashId("s"));
  }

  void test_create_string_value( void ) {
    StringValue v("hello");
    StringValue v2;

    assert_true(v.is_string());
    assert_true(v2.is_string());


    assert_equal("hello", v.str());
    assert_equal("", v2.str());
  }

  void test_create_std_string( void ) {
    std::string str("foo");
    Value v(str);

    assert_false(v.is_empty());
    assert_false(v.is_nil());
    assert_false(v.is_true());
    assert_false(v.is_false());
    assert_false(v.is_nil());
    assert_false(v.is_true());
    assert_false(v.is_false());
    assert_false(v.is_real());
    assert_true (v.is_string());
    assert_false(v.is_error());
    assert_false(v.is_hash());
    assert_false(v.is_any());

    assert_equal("foo", v.str());

    str.append("bar");
    assert_equal("foobar", str);
    assert_equal("foo", v.str());

    assert_equal("s", v.type_tag());
  }

  void test_create_with_char( void ) {
    Value v('s');

    assert_true(v.is_string());
    assert_equal("", v.str());
  }

  void test_create_with_TypeTag( void ) {
    Value v(TypeTag("s"));

    assert_true(v.is_string());
    assert_equal("", v.str());
  }

  void test_copy( void ) {
    Value v("foo");
    assert_equal(1, v.string_->ref_count());

    Value * v2 = new Value(v);
    assert_equal(v.string_, v2->string_);
    assert_equal(2, v.string_->ref_count());
    assert_true(v2->is_string());

    Value v3;

    assert_true(v3.is_empty());

    v3 = v;
    assert_true(v3.is_string());
    assert_equal(v.string_, v3.string_);
    assert_equal(3, v.string_->ref_count());

    assert_equal("foo", v.str());
    assert_equal("foo", v2->str());
    assert_equal("foo", v3.str());

    v.str() = "bar";
    assert_equal("bar", v.str());
    assert_equal("bar", v2->str());
    assert_equal("bar", v3.str());
    delete v2;
    assert_equal(2, v.string_->ref_count());
  }

  void test_set( void ) {
    Value v;

    assert_true(v.is_empty());
    v.set("foobar");
    assert_true(v.is_string());
    assert_equal("foobar", v.str());
  }

  void test_set_type_tag( void ) {
    Value v;
    v.set_type_tag("s");
    assert_true(v.is_string());
    assert_equal("", v.str());
  }

  void test_set_type( void ) {
    Value v;

    v.set_type(STRING_VALUE);
    assert_true(v.is_string());
    assert_equal("", v.str());
  }

  void test_to_json( void ) {
    Value v("cake");
    std::ostringstream os(std::ostringstream::out);
    os << v;
    assert_equal("\"cake\"", os.str());
    assert_equal("\"cake\"", v.to_json());
  }

  void test_from_json( void ) {
    Value v(Json("\"This is some \\\"super\\\" string !\""));
    assert_true(v.is_string());
    assert_equal("This is some \"super\" string !", v.str());
  }

  void test_from_json_single_quote( void ) {
    Value v(Json("'It took 25\" for \\'John\\' to \\\"get here !'"));
    assert_true(v.is_string());
    assert_equal("\"It took 25\\\" for 'John' to \\\"get here !\"", v.to_json());
  }

  void test_can_receive( void ) {
    Object object("foo", Oscit::string_io("info"));
    assert_false(object.can_receive(Value()));
    assert_true (object.can_receive(gNilValue));
    assert_true (object.can_receive(gBangValue));
    assert_false(object.can_receive(Value(1.23)));
    assert_true (object.can_receive(Value("foo")));
    assert_false(object.can_receive(Value(BAD_REQUEST_ERROR, "foo")));
    assert_false(object.can_receive(JsonValue("['','']")));
    assert_false(object.can_receive(HashValue()));
    assert_false(object.can_receive(MatrixValue(1,1)));
    assert_false(object.can_receive(MidiValue()));
  }

  void test_equal( void ) {
    Value a("one");
    Value b("one");
    assert_equal(a, b);
    a.set("two");
    assert_false(a == b);
    a.set(4);
    assert_false(a == b);
    a.set_nil();
    assert_false(a == b);
  }

  void test_stream_char( void ) {
    Value s("Hello");
    s << " " << "World!";
    assert_equal("Hello World!", s.str());
  }

  void test_stream_ints( void ) {
    Value s("");
    s << 44 << " " << 55;
    assert_equal("44 55", s.str());
  }

  void test_stream_double( void ) {
    Value s("");
    s << 4.4 << " " << 55;
    assert_equal("4.4 55", s.str());
  }

  void test_stream_other_value( void ) {
    Value s("");
    Value list(Json("[1,2,3]"));
    s << 4.4 << " " << list;
    assert_equal("4.4 [1, 2, 3]", s.str());
  }

  void test_create_varargs( void ) {
    FValue s("I am %i not '%s'.", 1337, "Superman");
    assert_equal("I am 1337 not 'Superman'.", s.str());
  }

  void test_split_char( void ) {
    Value s("one:.:/user/local/lib/rubyk:~/rubyk/lib");
    Value list = s.split(':');
    assert_equal("[\"one\", \".\", \"/user/local/lib/rubyk\", \"~/rubyk/lib\"]", list.to_json());
  }

  void test_split_const_char( void ) {
    Value s("one::.::/user/local/lib/rubyk::~/rubyk/lib");
    Value list = s.split("::");
    assert_equal("[\"one\", \".\", \"/user/local/lib/rubyk\", \"~/rubyk/lib\"]", list.to_json());
  }

  void test_split_not_a_string( void ) {
    Value s(45);
    Value list = s.split(':');
    assert_equal("[]", list.to_json());
  }

};