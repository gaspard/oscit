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
#include "mock/ref_counted_logger.h"

class ReferenceCountedTest : public TestHelper {
public:

  void test_ref_count_should_start_at_one(void) {
    Logger logger;
    RefCountedLogger obj("ref", &logger);

    assert_equal(1, obj.ref_count());
  }

  void test_ref_count_should_increase_on_acquire_or_retain(void) {
    Logger logger;
    RefCountedLogger obj("ref", &logger);
    ReferenceCounted::acquire(&obj);
    assert_equal(2, obj.ref_count());

    obj.retain();
    assert_equal(3, obj.ref_count());
  }

  void test_ref_count_should_decrease_on_release(void) {
    Logger logger;
    RefCountedLogger obj("ref", &logger);
    ReferenceCounted::acquire(&obj);
    ReferenceCounted::acquire(&obj);
    assert_equal(3, obj.ref_count());

    ReferenceCounted::release(&obj);
    assert_equal(2, obj.ref_count());

    obj.release();
    assert_equal(1, obj.ref_count());
  }

  void test_delete_on_ref_count_to_zero(void) {
    Logger logger;
    RefCountedLogger *obj = new RefCountedLogger("ref", &logger);

    ReferenceCounted::release(obj);
    assert_equal("[ref: destroyed]", logger.str());
    logger.str("");

    obj = new RefCountedLogger("ref", &logger);
    obj->release();
    assert_equal("[ref: destroyed]", logger.str());
  }
};