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

class TrueValueTest : public TestHelper
{
public:
  void test_is_true( void ) {

    assert_false(gTrueValue.is_empty());
    assert_false(gTrueValue.is_nil());
    assert_true (gTrueValue.is_true());
    assert_true (gTrueValue.is_bang());
    assert_false(gTrueValue.is_false());
    assert_false(gTrueValue.is_real());
    assert_false(gTrueValue.is_string());
    assert_false(gTrueValue.is_list());
    assert_false(gTrueValue.is_error());
    assert_false(gTrueValue.is_hash());
    assert_false(gTrueValue.is_matrix());
    assert_false(gTrueValue.is_any());

    assert_equal("T", gTrueValue.type_tag());
    assert_equal(TRUE_TYPE_TAG_ID, gTrueValue.type_id());

    uint type_id = H("T");
    assert_equal(TRUE_TYPE_TAG_ID, type_id);

  }

  void test_create_with_char( void ) {
    Value v('T');

    assert_true(v.is_true());
  }

  void test_create_with_TypeTag( void ) {
    Value v(TypeTag("T"));

    assert_true(v.is_true());
  }

  void test_copy( void ) {
    Value v('T');
    Value v2(v);
    Value v3(1.2);

    assert_true(v3.is_real());

    v3 = v;

    assert_true(v3.is_true());
    assert_true(v2.is_true());
  }

  void test_set( void ) {
    Value v(1.2);

    assert_true(v.is_real());
    v.set_true();
    assert_true(v.is_true());
  }

  void test_set_tag( void ) {
    Value v(1.2);

    v.set_type_tag("T");
    assert_true(v.is_true());
  }

  void test_set_type( void ) {
    Value v(1.2);

    v.set_type(TRUE_VALUE);
    assert_true(v.is_true());
  }

  void test_to_json( void ) {
    Value v('T');
    std::ostringstream os(std::ostringstream::out);
    os << v;
    assert_equal("true", os.str());
    assert_equal("true", v.to_json());
  }

  void test_from_json( void ) {
    Value v(Json("true"));
    assert_true(v.is_true());
  }

  void test_equal( void ) {
    Value a('T');
    Value b('T');
    assert_equal(a, b);
    a.set(3);
    assert_false(a == b);
    a.set_true();
    assert_equal(a, b);
  }
};