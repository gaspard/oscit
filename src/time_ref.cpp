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

#include "oscit/time_ref.h"

#include <sys/timeb.h> // ftime

namespace oscit {
struct TimeRef::TimeRefData : public timeb {};

TimeRef::TimeRef() {
  reference_ = new TimeRefData;
  ftime(reference_);
}

TimeRef::~TimeRef() {
  delete reference_;
}

/** Get current real time in [ms] since the time ref object was created.
 */
time_t TimeRef::elapsed() {
  TimeRefData t;
  /* FIXME: Use clock_gettime instead of ftime.

  int clock_gettime(clockid_t clock_id, struct timespec *tp);
  int clock_getres(clockid_t clock_id, struct timespec *res);
  */
  ftime(&t);
  return ((t.time - reference_->time) * 1000) + t.millitm - reference_->millitm;
}

} // oscit