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

#ifndef OSCIT_INCLUDE_OSCIT_METHOD_H_
#define OSCIT_INCLUDE_OSCIT_METHOD_H_
#include "oscit/object.h"

namespace oscit {

/** Function pointer to trigger a member method. */
typedef const Value (*member_method_t)(void *receiver, const Value &val);

/** Function pointer to trigger a class method. */
typedef const Value (*class_method_t )(Root *root, const Value &val);

/** Object instance to trigger a class method. */
class ClassMethod : public Object
{
 public:
  /** Class signature. */
  TYPED("Object.ClassMethod")

  /** Create a new object that will call a class method when "triggered". */
  ClassMethod(const std::string &name, class_method_t method, const Value &attrs) :
      Object(name, attrs), class_method_(method) {}

  /** Create a new object that will call a class method when "triggered". */
  ClassMethod(const char *name, class_method_t method, const Value &attrs) :
      Object(name, attrs), class_method_(method) {}

  /** Trigger: call the class method. */
  virtual const Value trigger(const Value &val) {
    return (*class_method_)(root_, val);
  }

 protected:
  class_method_t class_method_;   /**< Pointer on the class method. */
};


/** Prototype constructor for Method. */
struct MethodPrototype
{
  MethodPrototype(const char *name, member_method_t method, const Value &attrs, bool keep_last = false) :
      name_(name), member_method_(method), attrs_(attrs), keep_last_(keep_last) {}
  const char *    name_;
  member_method_t member_method_;
  Value           attrs_;
  bool            keep_last_;
};

/** Object instance to trigger a member method (stores a reference to the receiver). */
class Method : public Object
{
 public:
  TYPED("Object.Method")

  /** Create a new object that call a member method when "triggered". */
  Method(void *receiver, const char *name, member_method_t method, const Value &attrs) :
      Object(name, attrs), receiver_(receiver), member_method_(method) {}

  /** Create a new object that call a member method when "triggered". */
  Method(void *receiver, const std::string &name, member_method_t method, const Value &attrs) :
      Object(name, attrs), receiver_(receiver), member_method_(method) {}

  /** Prototype based constructor. */
  Method(void *receiver, const MethodPrototype &prototype) : Object(prototype.name_, prototype.attrs_, prototype.keep_last_),
      receiver_(receiver), member_method_(prototype.member_method_) {}

  /** Trigger: call the member method on the receiver. */
  virtual const Value trigger(const Value &val) {
    return (*member_method_)(receiver_,val);
  }

  /** Make a pointer to a member method. */
  template<class T, const Value(T::*Tmethod)(const Value&)>
  static const Value cast_method(void *receiver, const Value &val) {
    return (((T*)receiver)->*Tmethod)(val);
  }

  /** Make a pointer to a member method. */
  template<class R, class T, const Value(T::*Tmethod)(const Value&)>
  static const Value cast_method(void *receiver, const Value &val) {
    return (((R*)receiver)->*Tmethod)(val);
  }

  /** Make a pointer to a member method without return values. */
  template<class T, void(T::*Tmethod)(const Value&)>
  static const Value cast_method(void *receiver, const Value &val) {
    (((T*)receiver)->*Tmethod)(val);
    return gNilValue;
  }

  /** Make a pointer to a member method without return values. */
  template<class R, class T, void(T::*Tmethod)(const Value&)>
  static const Value cast_method(void *receiver, const Value &val) {
    (((R*)receiver)->*Tmethod)(val);
    return gNilValue;
  }
 protected:
  void *          receiver_;       /**< Object containing the method. */
  member_method_t member_method_;  /**< Pointer on a cast of the member method. */
};

template<class T, const Value(T::*Tmethod)(const Value&)>
class TMethod : public Method
{
 public:
  TYPED("Object.Method.TMethod")

  /** Create a new object that call a member method when "triggered". */
  TMethod(void *receiver, const char *name, const Value &attrs) :
      Method(receiver, name, &cast_method<T, Tmethod>, attrs) {}

  /** Create a new object that call a member method when "triggered". */
  TMethod(void *receiver, const std::string &name, const Value &attrs) :
      Method(receiver, name, &cast_method<T, Tmethod>, attrs) {}
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_METHOD_H_
