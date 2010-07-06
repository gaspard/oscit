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
#include "mock/dummy_object.h"

class TypeMetatMethodTest : public TestHelper
{
public:
  void test_range_type( void ) {
    Root root;
    root.adopt(new DummyObject("foo", 4.25));
    Value res;

    res = root.call(ATTRS_PATH, Value(""));
    assert_equal("", res[0].str());
    res = res[1];
    assert_equal("Container.", res[Attribute::INFO].str());

    res = root.call(ATTRS_PATH, Value("/foo"));
    assert_equal("/foo", res[0].str());
    res = res[1];
    assert_equal("H", res.type_tag());
    assert_equal("range", res[Attribute::TYPE][Attribute::NAME].str());
    assert_equal(0.0,   res[Attribute::TYPE][Attribute::MIN].r);
    assert_equal(127.0, res[Attribute::TYPE][Attribute::MAX].r);
    assert_equal(DUMMY_OBJECT_INFO, res[Attribute::INFO].str());

    res = root.call(ATTRS_PATH, Value("/blah"));
    assert_equal("/blah", res[0].str());
    res = res[1];
    assert_true(res.is_error());
    assert_equal(NOT_FOUND_ERROR, res.error_code());
  }

  void test_select_type( void ) {
    Root root;
    root.adopt(new DummyObject2("foo", "yuv"));
    Value res;

    res = root.call(ATTRS_PATH, Value("/foo"));
    assert_equal("/foo", res[0].str());
    res = res[1];
    assert_equal("H", res.type_tag());
    assert_equal("rgb,rgba,yuv",  res[Attribute::TYPE][Attribute::VALUES].str());
    assert_equal("Set color mode.", res[Attribute::INFO].str());
  }

  void test_any_type( void ) {
    Root root;
    root.adopt(new DummyObject("foo", 1.23, Attribute::any_io("This is the info string.")));
    Value res;
    res = root.call(ATTRS_PATH, Value("/foo"));
    assert_equal("/foo", res[0].str());
    res = res[1];
    assert_equal("*", res[Attribute::TYPE][Attribute::SIGNATURE].str());
    assert_equal("This is the info string.", res[Attribute::INFO].str()); // info
  }

  void test_list_type( void ) {
    Root root;
    // FIXME: replace JsonValue(...) by proper attributes
    // Attribute::attribute(info, name, signature) ?
    root.adopt(new DummyObject("Haddock", JsonValue("[\"\", 0.0]"), HashValue(Attribute::INFO, "Haddock").set(Attribute::TYPE, HashValue(Attribute::SIGNATURE, "sf").set(Attribute::NAME, "some list"))));
    Value res;
    res = root.call(ATTRS_PATH, Value("/Haddock"));
    assert_equal("/Haddock", res[0].str());
    res = res[1];
    assert_equal("sf", res[Attribute::TYPE][Attribute::SIGNATURE].str());
    assert_equal("some list", res[Attribute::TYPE][Attribute::NAME].str());
    assert_equal("Haddock", res[Attribute::INFO].str());
  }

  void test_hash_type( void ) {
    Root root;
    root.adopt(new DummyObject("dog_food", JsonValue("{lazy:\"dog\", silly:\"cats and mices\"}"), Attribute::hash_io("Blah blah.")));
    Value res;
    res = root.call(ATTRS_PATH, Value("/dog_food"));
    assert_equal("/dog_food", res[0].str());
    res = res[1];
    assert_equal("H", res[Attribute::TYPE][Attribute::SIGNATURE].str());
    assert_equal("hash", res[Attribute::TYPE][Attribute::NAME].str());
    assert_equal("Blah blah.", res[Attribute::INFO].str());
  }

  void test_matrix_type( void ) {
    Root root;
    root.adopt(new DummyObject("master_of_time", MatrixValue(1,5), Attribute::matrix_io("Stupid matrix.")));
    Value res;
    res = root.call(ATTRS_PATH, Value("/master_of_time"));
    assert_equal("/master_of_time", res[0].str());
    res = res[1];
    assert_equal("M", res[Attribute::TYPE][Attribute::SIGNATURE].str());
    assert_equal("matrix", res[Attribute::TYPE][Attribute::NAME].str());
    assert_equal("Stupid matrix.", res[Attribute::INFO].str());
  }

  void test_no_input( void ) {
    Root root;
    root.adopt(new Object("foo"));
    Value res;

    res = root.call(ATTRS_PATH, Value("/foo"));
    assert_equal("/foo", res[0].str());
    res = res[1];
    assert_equal("Container.", res[Attribute::INFO].str());
  }

  void test_type_with_nil( void ) {
    Root root(Attribute::no_io("This is the root node."));
    Value res;

    res = root.call(ATTRS_PATH, gNilValue);
    assert_true(res.is_nil());
  }
};