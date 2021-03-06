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
#include "oscit/rw_mutex.h"
#include <sstream>

#define RW_MUTEX_TEST_LOOP_COUNT 10000
#define RW_MUTEX_TEST_THREAD_READERS_COUNT 20
#define RW_MUTEX_TEST_THREAD_WRITERS_COUNT 2

int gRWMutexTest_a, gRWMutexTest_b;
RWMutex gRWMutexTest_mutex;
std::ostringstream gRWMutexTest_log(std::ostringstream::out);

void start_rwmutex_test_thread(Thread* runner) {
  gRWMutexTest_log << "[1:new]";
  runner->thread_ready();
  gRWMutexTest_mutex.lock();
  gRWMutexTest_log << "[1:lock]";
  runner->semaphore().release();
  gRWMutexTest_mutex.unlock();
  gRWMutexTest_log << "[1:unlock][1:end]";
}

static void lock_rw_method(Thread* runner) {
  ScopedWrite lock(gRWMutexTest_mutex);
  gRWMutexTest_log << "[1:lock]";
  runner->semaphore().release();
}

static void start_rwmutex_test_thread_scoped_lock(Thread* runner) {
  gRWMutexTest_log << "[1:new]";
  runner->thread_ready();
  lock_rw_method(runner);
  gRWMutexTest_log << "[1:unlock][1:end]";
}

class RWMutexTest : public TestHelper
{
public:

  void should_give_exclusive_access_on_write_lock( void ) {
    int errors[RW_MUTEX_TEST_THREAD_READERS_COUNT];
    gRWMutexTest_a = 0;
    gRWMutexTest_b = 0;

    // READY, SET, ...
    gRWMutexTest_mutex.lock();

    // create n threads that will increment and decrement the counter
    pthread_t thread[RW_MUTEX_TEST_THREAD_READERS_COUNT];

    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_READERS_COUNT; ++i) {
      pthread_create( &thread[i], NULL, RWMutexTest::reader_thread, (void*)(errors + i));
    }

    // create writer threads
    pthread_t wthread[RW_MUTEX_TEST_THREAD_WRITERS_COUNT];
    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_READERS_COUNT; ++i) {
      if (i % 2 == 0) {
        pthread_create( &wthread[i], NULL, RWMutexTest::writer1_thread, NULL);
      } else {
        pthread_create( &wthread[i], NULL, RWMutexTest::writer2_thread, NULL);
      }
    }


    // GO !
    gRWMutexTest_mutex.unlock();

    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_READERS_COUNT; ++i) {
      pthread_join(thread[i], NULL);
    }

    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_WRITERS_COUNT; ++i) {
      pthread_join(wthread[i], NULL);
    }

    int total_errors = 0;
    for (size_t i=0; i < RW_MUTEX_TEST_THREAD_READERS_COUNT; ++i) {
      total_errors += errors[i];
    }
    assert_equal(0, total_errors);
  }

  void test_bad_unlock( void ) {
    RWMutex mutex;
    mutex.unlock();
    // should not raise any error
    assert_true( true ); // passed !
  }

  void test_two_threads( void ) {
    Thread runner;
    gRWMutexTest_mutex.lock();
    gRWMutexTest_log << "[0:lock]";
    // create new thread (will try to get hold of the lock)
    runner.start_thread<start_rwmutex_test_thread>(NULL);
    // release lock() --> other gets hold of it
    gRWMutexTest_log << "[0:unlock]";
    gRWMutexTest_mutex.unlock();
    runner.semaphore().acquire();
    gRWMutexTest_mutex.lock();
    gRWMutexTest_log << "[0:lock]";
    gRWMutexTest_mutex.unlock();
    runner.join();
    gRWMutexTest_log << "[joined]";

    assert_equal("[0:lock][1:new][0:unlock][1:lock][1:unlock][1:end][0:lock][joined]", gRWMutexTest_log.str());
  }

  void test_two_threads_scoped_write_lock( void ) {
    gRWMutexTest_log.str("");
    Thread runner;
    {
      ScopedWrite lock(gRWMutexTest_mutex);
      gRWMutexTest_log << "[0:lock]";
      // create new thread (will try to get hold of the lock)
      runner.start_thread(start_rwmutex_test_thread_scoped_lock, NULL);
      // release lock() --> other gets hold of it
      gRWMutexTest_log << "[0:unlock]";
    }
    runner.semaphore().acquire();
    {
      ScopedWrite lock(gRWMutexTest_mutex);
      gRWMutexTest_log << "[0:lock]";
    }
    runner.join();
    gRWMutexTest_log << "[joined]";

    assert_equal("[0:lock][1:new][0:unlock][1:lock][1:unlock][1:end][0:lock][joined]", gRWMutexTest_log.str());
  }

  //void test_two_threads_scoped_lock( void ) {
  //  Thread runner;
  //  {
  //    ScopedLock lock(gRWMutexTest_mutex);
  //    gRWMutexTest_log << "[0:lock]";
  //    // create new thread (will try to get hold of the lock)
  //    runner.start_thread(start_rwmutex_test_thread_scoped_lock, NULL);
  //    // release lock() --> other gets hold of it
  //    gRWMutexTest_log << "[0:unlock]";
  //  }
  //  runner.semaphore().acquire();
  //  {
  //    ScopedLock lock(gRWMutexTest_mutex);
  //    gRWMutexTest_mutex.lock();
  //    gRWMutexTest_log << "[0:lock]";
  //  }
  //  runner.join();
  //  gRWMutexTest_log << "[joined]";
  //
  //  assert_equal("[0:lock][1:new][0:unlock][1:lock][1:unlock][1:end][0:lock][joined]", gRWMutexTest_log.str());
  //}

  static void *writer1_thread( void *data ) {
    // this thread periodically writes new values to the shared data
    for(size_t i = 0; i < RW_MUTEX_TEST_LOOP_COUNT; ++i) {
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
      { ScopedRead lock(gRWMutexTest_mutex);
        int a = gRWMutexTest_a;
        int b = gRWMutexTest_b;
        if (a != b) {
          ++*errors;
        }
      }
    }
    return NULL;
  }
};
