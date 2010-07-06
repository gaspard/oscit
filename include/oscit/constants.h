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
  static const char * const TYPE;
  static const char * const INFO;
  static const char * const SIGNATURE;
  static const char * const VALUES;
  static const char * const NAME;
  static const char * const MIN;
  static const char * const MAX;

  static Value &default_io() {
    static Value default_attrs(no_io("Container."));
    return default_attrs;
  }

  static Value no_io(const char *info) {
    return HashValue(
      INFO, info);
      // No name, no signature
  }

  static Value bang_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "bang").set(
        SIGNATURE, "T"   )
      );
  }

  static Value real_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "real").set(
        SIGNATURE, "f"   )
      );
  }

  static Value string_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "string").set(
        SIGNATURE, "s"     )
      );
  }

  static Value range_io(Real min, Real max, const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "range").set(
        SIGNATURE, "f"    ).set(
        MIN,       min    ).set(
        MAX,       max    )
      );
  }

  static Value select_io(const char *values, const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "select").set(
        SIGNATURE, "s"     ).set(
        VALUES,    values)
      );
  }

  static Value hash_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "hash").set(
        SIGNATURE, "H"   )
      );
  }

  static Value matrix_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "matrix").set(
        SIGNATURE, "M"     )
      );
  }

  static Value midi_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "midi").set(
        SIGNATURE, "m")
      );
  }

  static Value any_io(const char *info) {
    return HashValue(
      INFO, info).set(
      TYPE, HashValue(
        NAME,      "any").set(
        SIGNATURE, "*"  )
      );
  }
};

} // oscit
#endif  // OSCIT_INCLUDE_OSCIT_CONSTANTS_H_
