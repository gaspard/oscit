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
#include "oscit/rw_mutex.h"
#include <sstream>

#define RW_MUTEX_TEST_LOOP_COUNT 2000
#define RW_MUTEX_TEST_THREAD_COUNT 200

int gRWMutexTest_a, gRWMutexTest_b;
RWMutex gRWMutexTest_mutex;

class RWMutexTest : public TestHelper
{
public:

  void should_give_exclusive_access_on_write_lock( void ) {
    int errors[RW_MUTEX_TEST_THREAD_COUNT];
    gRWMutexTest_a = 0;
    gRWMutexTest_b = 0;
    pthread_t writer1;
    pthread_t writer2;

    // READY, SET, ...
    gRWMutexTest_mutex.lock();

    // create n threads that will increment and decrement the counter
    pthread_t thread[RW_MUTEX_TEST_THREAD_COUNT];

    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_COUNT; ++i) {
      pthread_create( &thread[i], NULL, RWMutexTest::reader_thread, (void*)(errors + i));
    }

    // create writer threads
    pthread_create( &writer1, NULL, RWMutexTest::writer1_thread, NULL);
    pthread_create( &writer2, NULL, RWMutexTest::writer2_thread, NULL);

    // GO !
    gRWMutexTest_mutex.unlock();

    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_COUNT; ++i) {
      pthread_join(thread[i], NULL);
    }

    pthread_join(writer1, NULL);
    pthread_join(writer2, NULL);

    int total_errors = 0;
    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_COUNT; ++i) {
      total_errors += errors[i];
    }
    assert_equal(0, total_errors);
  }

  static void *writer1_thread( void *data ) {
    // this thread periodically writes new values to the shared data
    for(size_t i = 0; i < RW_MUTEX_TEST_LOOP_COUNT; ++i) {
      printf("+");
      fflush(stdout);
      { ScopedWrite lock(gRWMutexTest_mutex);
        ++gRWMutexTest_a;
        ++gRWMutexTest_b;
      }
    }
    return NULL;
  }

  static void *writer2_thread( void *data ) {
    // this thread periodically writes new values to the shared data
    for(size_t i = 0; i < RW_MUTEX_TEST_LOOP_COUNT; ++i) {
      printf("=");
      fflush(stdout);
      { ScopedWrite lock(gRWMutexTest_mutex);
        --gRWMutexTest_a;
        --gRWMutexTest_b;
      }
    }
    return NULL;
  }

  static void *reader_thread( void *data ) {
    int *errors = (int*)data;
    *errors = 0;
    // this thread compares member values gRWMutexTest_a and gRWMutexTest_b and logs errors in data
    for(size_t i = 0; i < RW_MUTEX_TEST_LOOP_COUNT; ++i) {
      printf(".");
      fflush(stdout);
      { ScopedRead lock(gRWMutexTest_mutex);
        if (gRWMutexTest_a != gRWMutexTest_b) {
          ++*errors;
        }
      }
    }
    return NULL;
  }
};
