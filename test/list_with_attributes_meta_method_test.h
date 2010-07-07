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

class ListWithAttributesMetaMethodTest : public TestHelper
{
public:
  /* Bernoulli family
  /Nikolaus
  /Nikolaus/Jacob
  /Nikolaus/Nikolaus
  /Nikolaus/Johann
  /Nikolaus/Johann/Nicolaus
  /Nikolaus/Johann/Daniel
  /Nikolaus/Johann/Johann
   */

  void test_list_with_attributes_on_root( void ) {
    Root root;
    root.adopt(new DummyObject("mode", "rgb", Attribute::select_io("This is a menu.", "rgb, yuv")));
    root.adopt(new DummyObject("tint", 45.0, Attribute::range_io("This is a slider from 1 to 127.", 1, 127)));
    Value res = root.list_with_attributes();
    // .error, .info, etc
    //
    assert_equal("sHsHsHsHsHsHsHsH", res.type_tag());
    assert_equal("\".error\"", res[0].to_json());
    assert_equal("\".info\"", res[2].to_json());
  }

  void test_list_with_attributes( void ) {
    Root root;
    Object * tmp = root.adopt(new Object("monitor"));
    tmp->adopt(new DummyObject("mode", "rgb", Attribute::select_io("This is a menu.", "rgb, yuv")));
    tmp->adopt(new DummyObject("tint", 45.0, Attribute::range_io("This is a slider from 1 to 127.", 1, 127)));
    Value reply, res;

    reply = root.call(LIST_WITH_ATTRIBUTES_PATH, Value(""));
    assert_equal("", reply[0].str());
    res = reply[1];
    assert_true(res.is_list());
    assert_equal(2 * 7, res.size()); // 7 methods, 7 keys

    assert_equal(Url(ERROR_PATH).name(), res[0].str());
    assert_equal(Url(INFO_PATH).name(), res[2].str());
    assert_equal(Url(LIST_PATH).name(), res[4].str());
    assert_equal(Url(LIST_WITH_ATTRIBUTES_PATH).name(), res[6].str());
    assert_equal(Url(ATTRS_PATH).name(), res[8].str());
    assert_equal(Url(TREE_PATH).name(), res[10].str());
    assert_equal("monitor/", res[12].str());

    reply = root.call(LIST_WITH_ATTRIBUTES_PATH, Value("/monitor"));
    assert_equal("/monitor", reply[0].str());
    res = reply[1];
    assert_true(res.is_list());
    assert_equal(2 * 2, res.size());
    assert_equal("mode",    res[0].str());
    assert_equal("tint",    res[2].str());
  }

  void test_list_with_attributes_on_empty( void ) {
    Root root;
    root.adopt(new Object("Nikolaus"));
    Value res;

    res = root.call(LIST_WITH_ATTRIBUTES_PATH, Value("/Nikolaus"));
    assert_equal("/Nikolaus", res[0].str());
    res = res[1];
    assert_true(res.is_list());
    assert_equal(0,  res.size());
  }

  void test_list_with_attributes_with_nil( void ) {
    Root root(Attribute::no_io("This is the root node."));
    Value res;

    res = root.call(LIST_WITH_ATTRIBUTES_PATH, gNilValue);
    assert_true(res.is_nil());
  }
};