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

#ifndef OSCIT_INCLUDE_OSCIT_C_THASH_H_
#define OSCIT_INCLUDE_OSCIT_C_THASH_H_

#include "oscit/thash.h"
#include "oscit/rw_mutex.h"

namespace oscit {


/** The Concurrent Hash template is a Hash with a read-write mutex.
 * The class does not lock/unlock the mutex, this is the responsability
 * of the class using the hash. The reason for this is that it is often
 * necessary to read-lock a little longer then the "get" call because you
 * might need to reference increment or lock the found object before
 * releasing the lock:
 *
 * @code
 * {
 *   ScopedRead lock(objects_);
 *   if (objects_.get(key, &target)) {
 *     target->retain();
 *   } else {
 *     return;
 *   }
 * }
 * ..
 * target->release();
 * @endcode
 * K is the key class, T is the object class
 */
template <class K, class T>
class CTHash : public THash<K,T>, public RWMutex {
public:
  CTHash(unsigned int size) : THash<K,T>(size) {}

  // copy constructor
  CTHash(const CTHash& other) : THash<K,T>(other.size_), RWMutex() {
    copy(other);
  }

  CTHash& operator=(const CTHash& other) {
    copy(other);
    return *this;
  }
};

}

#endif // OSCIT_INCLUDE_OSCIT_C_THASH_H_
