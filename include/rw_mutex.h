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

namespace oscit {

/** This class is a wrapper around Semaphore to implement a read/write mutex.
 * It should be used when there can be many reading threads but a single writer
 * (without readers) accessing the resource at the same time.
 */
class RWMutex : public Semaphore {
public:
  RWMutex(uint max_reader_count = 10) : Semaphore(max_reader_count) {}

  inline void lock() {
    acquire_all();
  }

  inline void unlock() {
    release_all();
  }

  inline void read_lock() {
    acquire();
  }

  inline void read_unlock() {
    release();
  }
};

}  // oscit

#endif // OSCIT_INCLUDE_OSCIT_RW_MUTEX_H_