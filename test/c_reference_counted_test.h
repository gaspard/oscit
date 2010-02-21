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
#include "mock/c_ref_counted_logger.h"

#define CREF_COUNTED_TEST_THREAD_COUNT 100
#define CREF_COUNTED_TEST_LOOP_SIZE 100000

static void *cref_counted_test_retain_release( void *data ) {
  CRefCountedLogger *ref = (CRefCountedLogger*)data;
  for (int i=0; i < CREF_COUNTED_TEST_LOOP_SIZE; ++i) {
    ref->retain();
    ref->release();
  }
  return NULL;
}

class CReferenceCountedTest : public TestHelper {
public:

  void should_work_concurrently(void) {
    Logger logger;
    CRefCountedLogger *object = new CRefCountedLogger("ref", &logger);

    // create n threads that will increment and decrement the counter
    pthread_t thread[CREF_COUNTED_TEST_THREAD_COUNT];

    for (size_t i=0; i < CREF_COUNTED_TEST_THREAD_COUNT; ++i) {
      pthread_create( &thread[i], NULL, cref_counted_test_retain_release, (void*)object);
    }

    for (size_t i=0; i < CREF_COUNTED_TEST_THREAD_COUNT; ++i) {
      pthread_join(thread[i], NULL);
    }

    assert_equal("", logger.str());
    assert_equal(1, object->ref_count());

    object->release();
    assert_equal("[ref: destroyed]", logger.str());
  }

  void test_ref_count_should_start_at_one(void) {
    Logger logger;
    CRefCountedLogger obj("ref", &logger);

    assert_equal(1, obj.ref_count());
  }

  void test_ref_count_should_increase_on_acquire_or_retain(void) {
    Logger logger;
    CRefCountedLogger obj("ref", &logger);
    ReferenceCounted::acquire(&obj);
    assert_equal(2, obj.ref_count());

    obj.retain();
    assert_equal(3, obj.ref_count());
  }

  void test_ref_count_should_decrease_on_release(void) {
    Logger logger;
    CRefCountedLogger obj("ref", &logger);

    // You can use either CReferenceCounted::acquire or ReferenceCounted::acquire
    CReferenceCounted::acquire(&obj);
    CReferenceCounted::acquire(&obj);
    assert_equal(3, obj.ref_count());

    CReferenceCounted::release(&obj);
    assert_equal(2, obj.ref_count());

    obj.release();
    assert_equal(1, obj.ref_count());
  }

  void test_delete_on_ref_count_to_zero(void) {
    Logger logger;
    CRefCountedLogger *obj = new CRefCountedLogger("ref", &logger);

    ReferenceCounted::release(obj);
    assert_equal("[ref: destroyed]", logger.str());
    logger.str("");

    obj = new CRefCountedLogger("ref", &logger);
    obj->release();
    assert_equal("[ref: destroyed]", logger.str());
  }
};