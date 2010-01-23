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
#include "oscit/atomic_counter.h"

#define ATOMIC_COUNTER_TEST_THREAD_COUNT 100
#define ATOMIC_COUNTER_TEST_LOOP_SIZE 100000

static void *atomic_counter_test_play( void *data ) {
  AtomicCounter *counter = (AtomicCounter*)data;
  for (int i=0; i < ATOMIC_COUNTER_TEST_LOOP_SIZE; ++i) {
    counter->increment();
    counter->decrement();
  }
  return NULL;
}


class AtomicCounterTest : public TestHelper {
public:

  void test_counter_should_reflect_count(void) {
    AtomicCounter counter;

    // create n threads that will increment and decrement the counter
    pthread_t thread[ATOMIC_COUNTER_TEST_THREAD_COUNT];

    for (size_t i=0; i < ATOMIC_COUNTER_TEST_THREAD_COUNT; ++i) {
      pthread_create( &thread[i], NULL, atomic_counter_test_play, (void*)&counter);
    }

    for (size_t i=0; i < ATOMIC_COUNTER_TEST_THREAD_COUNT; ++i) {
      pthread_join(thread[i], NULL);
    }

    assert_equal(0, counter.count());
  }
  //
};