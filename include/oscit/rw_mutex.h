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

#ifndef OSCIT_INCLUDE_OSCIT_RW_MUTEX_H_
#define OSCIT_INCLUDE_OSCIT_RW_MUTEX_H_

#include "oscit/semaphore.h"
#include "oscit/non_copyable.h"

#include <string.h>  // strerror

namespace oscit {

#if 0

/** Dummy to test concurrency failures.
 */
class RWMutex : private NonCopyable {
public:
  RWMutex() {}

  inline void lock() {}

  inline void unlock() {}

  inline void read_lock() {}

  inline void read_unlock() {}

};

#else
#if 1

// POSIX pthread_rwlock
/** This class implements a read/write mutex.
 * It should be used when there are many more reads then writes, otherwize a
 * normal mutex is faster.
 */
class RWMutex {
public:
  RWMutex() {
    int status = pthread_rwlock_init(&rwlock_, NULL);
    if (status != 0) {
      fprintf(stderr, "Could not initialize rwlock (%s)\n", strerror(status));
    }
  }

  ~RWMutex() {
    int status = pthread_rwlock_destroy(&rwlock_);
    if (status != 0) {
      fprintf(stderr, "Could not destroy rwlock (%s)\n", strerror(status));
    }
  }

  /** Gain exclusive access to resource (write).
   */
  inline void lock() {
    pthread_rwlock_wrlock(&rwlock_);
  }

  /** Release exclusive access to resource.
   */
  inline void unlock() {
    pthread_rwlock_unlock(&rwlock_);
  }

  /** Gain read-only access to resource.
   */
  inline void read_lock() const {
    pthread_rwlock_rdlock(&rwlock_);
  }

  /** Release read-only access to resource.
   */
  inline void read_unlock() const {
    // YES, this const_cast is ugly, but that's the only way to allow READ lock on const.
    pthread_rwlock_unlock(&rwlock_);
  }
private:
  /** Pthread readers-writer lock.
   * We need to make it mutable so that we can access it from const 'read_unlock'.
   */
  mutable pthread_rwlock_t rwlock_;
};

#else
// Homebrewed RWMutex using a semaphore
// On Windows, we can use ReaderWriterLock.
// If we still need to build our own, we could use conditional variable with atomic
// reader count.


/** This class implements a read/write mutex.
 * It should be used when there are many more reads then writes, otherwize a
 * normal mutex is faster.
 */
class RWMutex : private Semaphore {
public:
  RWMutex(size_t max_reader_count = 16) : Semaphore(max_reader_count) {}

  /** Gain exclusive access to resource (write).
   */
  inline void lock() {
    acquire_all();
  }

  /** Release exclusive access to resource.
   */
  inline void unlock() {
    release_all();
  }

  /** Gain read-only access to resource.
   */
  inline void read_lock() {
    acquire();
  }

  /** Release read-only access to resource.
   */
  inline void read_unlock() {
    release();
  }

};

#endif

#endif // dummy

/** Scoped RWMutex Lock for exclusive access to a resource.
 */
class ScopedWrite : private NonCopyable {
public:
  ScopedWrite(RWMutex *mutex) : mutex_ptr_(mutex) {
    mutex_ptr_->lock();
  }

  ScopedWrite(RWMutex &mutex) : mutex_ptr_(&mutex) {
    mutex_ptr_->lock();
  }

  ~ScopedWrite() {
    mutex_ptr_->unlock();
  }

private:
  RWMutex *mutex_ptr_;
};

/** Lock for read-only access to a resource.
 */
class ScopedRead : private NonCopyable {
public:
  ScopedRead(const RWMutex *mutex) : mutex_ptr_(mutex) {
    mutex_ptr_->read_lock();
  }

  ScopedRead(const RWMutex &mutex) : mutex_ptr_(&mutex) {
    mutex_ptr_->read_lock();
  }

  ~ScopedRead() {
    mutex_ptr_->read_unlock();
  }

private:
  const RWMutex *mutex_ptr_;
};

}  // oscit

#endif  // OSCIT_INCLUDE_OSCIT_RW_MUTEX_H_
