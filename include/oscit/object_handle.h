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

#include "oscit/object.h"

#ifndef OSCIT_INCLUDE_OSCIT_OBJECT_HANDLE_H_
#define OSCIT_INCLUDE_OSCIT_OBJECT_HANDLE_H_

namespace oscit {
/** This is a handle that automatically retains and releases
 * the object it is holding.
 * It should be used as a specialized ScopedRetain for Objects.
 */
class ObjectHandle {
public:
  ObjectHandle(Object *object) : object_(object) {
    retain();
  }

  ObjectHandle() : object_(NULL) {}

  // no copy for the moment
  //
  // ObjectHandle(const ObjectHandle &other) : object_(other.object_) {
  //   if (object_) retain();
  // }
  //

  void operator=(const ObjectHandle &other) {
    release();
    object_ = other.object_;
    if (object_) retain();
  }

  void operator=(Object *object) {
    hold(object);
  }

  ~ObjectHandle() {
    release();
  }

  void hold(Object *object) {
    if (object == object_) return;
    release();
    object_ = object;
    if (object_) retain();
  }

  /** This code is meant to run on empty ScopedRetain handles, if you
   * need to hold an object longer, you should use a temporary to get
   * the value and copy it.
   */
  void hold_fast(Object *object) {
    assert(!object_);
    assert(object);
    object_ = object;
    retain();
  }

  /** Make sure you are holding an object before calling
   * this !
   */
  Object &operator*() {
    return *object_;
  }

  /** Make sure you are holding an object before calling
   * this !
   */
  Object *operator->() {
    return object_;
  }

  template<class T>
  T *type_cast() {
    return TYPE_CAST(T, object_);
  }

  Object *ptr() {
    return object_;
  }

protected:
  Object *object_;

  void release() {
    if (object_) object_->release();
  }

  void retain() {
    object_->retain();
  }
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_OBJECT_HANDLE_H_
