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

#ifndef OSCIT_INCLUDE_OSCIT_C_REFERENCE_COUNTED_H_
#define OSCIT_INCLUDE_OSCIT_C_REFERENCE_COUNTED_H_

#include <assert.h>

#include "oscit/atomic_counter.h"
#include "oscit/non_copyable.h"

namespace oscit {

/** Maintains a thread-safe reference count of an element.
 * When the reference count reaches zero, the object is deleted.
 * The reference count of an object starts at 1 and is incremented on <tt>retain</tt> and
 * decremented on <tt>release</tt>.
 * The 'C' in the name stands for 'concurrent'.
 */
class CReferenceCounted : private NonCopyable {
 public:
  CReferenceCounted() : ref_count_(1) {}

  virtual ~CReferenceCounted() {}

  void retain() {
    ref_count_.increment();
  }

  void release() {
    if (ref_count_.decrement() == 0) delete this;
  }

  int32_t ref_count() {
    return ref_count_.count();
  }

  template<class T>
  static T* acquire(T *elem) {
    elem->retain();
    return elem;
  }

  template<class T>
  static T* release(T *elem) {
    elem->release();
    return NULL;
  }


 private:
  AtomicCounter ref_count_;
};

class ScopedRetain : private NonCopyable {
public:
  ScopedRetain(CReferenceCounted *object) : object_(object) {
    retain();
  }

  ScopedRetain() : object_(NULL) {}

  ~ScopedRetain() {
    release();
  }

  void hold(CReferenceCounted *object) {
    assert(!object_);
    object_ = object;
    retain();
  }

private:
  CReferenceCounted *object_;

  void release() {
    if (object_) object_->release();
  }

  void retain() {
    object_->retain();
  }
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_C_REFERENCE_COUNTED_H_
