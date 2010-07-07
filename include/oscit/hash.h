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

#ifndef OSCIT_INCLUDE_OSCIT_HASH_H_
#define OSCIT_INCLUDE_OSCIT_HASH_H_

#include <list>
#include <string>

#include "oscit/thash.h"
#include "oscit/reference_counted.h"

namespace oscit {

class Value;
typedef std::list<std::string>::const_iterator HashIterator;

/** A Hash is just a reference counted THash<std::string,Value>.
 * This class also has a copy-on-write method called 'detach'.
 */
class Hash : public ReferenceCounted, public THash<std::string,Value> {
public:
  typedef std::list<std::string>::const_iterator const_iterator;
  explicit Hash(size_t size) : THash<std::string,Value>(size) {}

  /** Make a private copy to be modified in case the Hash is shared.
   */
  Hash *detach() {
    if (ref_count() > 1) {
      // copy on write
      Hash *new_hash = new Hash(*this);
      ReferenceCounted::release(this);
      return new_hash;
    } else {
      return this;
    }
  }

  /** Return true if any of the hash values is or contains an error.
   */
  bool contains_error();
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_HASH_H_
