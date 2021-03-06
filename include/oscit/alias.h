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

#ifndef OSCIT_INCLUDE_OSCIT_ALIAS_H_
#define OSCIT_INCLUDE_OSCIT_ALIAS_H_

#include "oscit/root.h"

namespace oscit {

/** This object triggers another object's trigger when called. It's used in Group to expose functionalities.
 */
class Alias : public Object
{
public:
  /** Class signature. */
  TYPED("Object.Alias")

  Alias() : original_(NULL) {}

  Alias(const char *name, Object *object)
    : Object(name),
      original_(NULL) {
    set_original(object);
  }

  Alias(const std::string &name, Object *object) : Object(name),
    original_(NULL) {
    set_original(object);
  }

  virtual const Value trigger(const Value &val) {
    return original_ ? original_->trigger(val) : gNilValue;
  }

private:
  void set_original(Object *object) {
    if (original_) {
      original_->on_delete().disconnect(this);
    }

    original_ = object;
    original_->on_delete().connect(this, &Alias::original_deleted);
    attributes_ = HashValue(Oscit::TYPE, object->type()).set(Oscit::INFO, object->info());
    sync_type_id();
  }

  void original_deleted(const Value &val) {
    delete this;
  }

protected:
  /** Original object pointed to by the alias.
   */
  Object *original_;
};

} // oscit
#endif // OSCIT_INCLUDE_OSCIT_ALIAS_H_
