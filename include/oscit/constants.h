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


#ifndef OSCIT_INCLUDE_OSCIT_CONSTANTS_H_
#define OSCIT_INCLUDE_OSCIT_CONSTANTS_H_

#include "oscit/values.h"

namespace oscit {

/** This is a dummy class to hold attribute globals.
 */
class Attribute {
public:
  /** Root attribute key for a human readable string describing the object.
   */
  static const char * const INFO;

  /** Root attribute key for type information on the object.
   */
  static const char * const TYPE;
  static const char * const SIGNATURE;
  static const char * const VALUES;
  static const char * const NAME;
  static const char * const MIN;
  static const char * const MAX;

  /** Root attribute key for view attributes (representation of object in a view).
   */
  static const char * const VIEW;
  static const char * const WIDGET;
  static const char * const HUE;
  static const char * const POS_X;
  static const char * const POS_Y;
  static const char * const WIDTH;
  static const char * const HEIGHT;

  /** Root attribute key for object's class url.
   */
  static const char * const CLASS;

  static Value &default_io() {
    static Value default_attrs(no_io("Container."));
    return default_attrs;
  }

  static Value no_io(const char *info) {
    return HashValue(
      INFO, info);
      // No name, no signature
  }

  static Value io(const char *info, const char *name, const char *signature) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      name).set(
        SIGNATURE, signature)
      );
  }

  static Value bang_io(const char *info) {
    return io(info, "bang", "T");
  }

  static Value real_io(const char *info) {
    return io(info, "real", "f");
  }

  static Value string_io(const char *info) {
    return io(info, "string", "s");
  }

  static Value range_io(const char *info, Real min, Real max) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "range").set(
        SIGNATURE, "f"    ).set(
        MIN,       min    ).set(
        MAX,       max    )
      );
  }

  static Value select_io(const char *info, const char *values) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "select").set(
        SIGNATURE, "s"     ).set(
        VALUES,    values)
      );
  }

  static Value hash_io(const char *info) {
    return io(info, "hash", "H");
  }

  static Value matrix_io(const char *info) {
    return io(info, "matrix", "M");
  }

  static Value midi_io(const char *info) {
    return io(info, "midi", "m");
  }

  static Value any_io(const char *info) {
    return io(info, "any", "*");
  }
};

} // oscit
#endif  // OSCIT_INCLUDE_OSCIT_CONSTANTS_H_
