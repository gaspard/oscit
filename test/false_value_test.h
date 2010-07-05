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

class FalseValueTest : public TestHelper
{
public:
  void test_is_false( void ) {

    assert_false(gFalseValue.is_empty());
    assert_false(gFalseValue.is_nil());
    assert_false(gFalseValue.is_true());
    assert_true (gFalseValue.is_false());
    assert_false(gFalseValue.is_real());
    assert_false(gFalseValue.is_string());
    assert_false(gFalseValue.is_list());
    assert_false(gFalseValue.is_error());
    assert_false(gFalseValue.is_hash());
    assert_false(gFalseValue.is_matrix());
    assert_false(gFalseValue.is_any());

    assert_equal("F", gFalseValue.type_tag());

    assert_equal("F", gFalseValue.type_tag());
    assert_equal(FALSE_TYPE_TAG_ID, gFalseValue.type_id());

    assert_equal(FALSE_TYPE_TAG_ID, hashId("F"));

  }

  void test_create_with_char( void ) {
    Value v('F');

    assert_true(v.is_false());
  }

  void test_create_with_TypeTag( void ) {
    Value v(TypeTag("F"));

    assert_true(v.is_false());
  }

  void test_copy( void ) {
    Value v('F');
    Value v2(v);
    Value v3(1.2);

    assert_true(v3.is_real());

    v3 = v;

    assert_true(v3.is_false());
    assert_true(v2.is_false());
  }

  void test_set( void ) {
    Value v(1.2);

    assert_true(v.is_real());
    v.set_false();
    assert_true(v.is_false());
  }

  void test_set_tag( void ) {
    Value v(1.2);

    v.set_type_tag("F");
    assert_true(v.is_false());
  }

  void test_set_type( void ) {
    Value v(1.2);

    v.set_type(FALSE_VALUE);
    assert_true(v.is_false());
  }

  void test_to_json( void ) {
    Value v('F');
    std::ostringstream os(std::ostringstream::out);
    os << v;
    assert_equal("false", os.str());
    assert_equal("false", v.to_json());
  }

  void test_from_json( void ) {
    Value v(Json("false"));
    assert_true(v.is_false());
  }

  void test_equal( void ) {
    Value a('F');
    Value b('F');
    assert_equal(a, b);
    a.set(3);
    assert_false(a == b);
    a.set_false();
    assert_equal(a, b);
  }
};