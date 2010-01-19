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

#ifndef OSCIT_INCLUDE_OSCIT_SCOPED_LOCKS_H_
#define OSCIT_INCLUDE_OSCIT_SCOPED_LOCKS_H_

#include "oscit/mutex.h"
#include "oscit/rw_mutex.h"

namespace oscit {

/** Lock for exclusive access to a resource.
 */
template<class T>
class ScopedLock : private NonCopyable {
public:
  ScopedLock(T *mutex) : mutex_ptr_(mutex) {
    mutex_ptr_->lock();
  }

  ScopedLock(T &mutex) : mutex_ptr_(&mutex) {
    mutex_ptr_->lock();
  }

  ~ScopedLock() {
    mutex_ptr_->unlock();
  }
 private:
  T *mutex_ptr_;
};

/** Lock for read-only access to a resource.
 */
class ScopedRead : public NonCopyable {
public:
  ScopedRead(T *mutex) : mutex_ptr_(mutex) {
    mutex_ptr_->read_lock();
  }

  ScopedRead(T &mutex) : mutex_ptr_(&mutex) {
    mutex_ptr_->read_lock();
  }

  ~ScopedRead() {
    mutex_ptr_->read_unlock();
  }

private:
  T *mutex_ptr_;
};

}  // oscit

#endif // OSCIT_INCLUDE_OSCIT_RW_MUTEX_H_