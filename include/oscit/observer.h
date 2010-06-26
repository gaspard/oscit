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

#ifndef OSCIT_INCLUDE_OSCIT_OBSERVER_H_
#define OSCIT_INCLUDE_OSCIT_OBSERVER_H_

#include <list>
#include <iostream>

#include "oscit/c_tlist.h"

namespace oscit {

class Signal;

/** In order to connect to signals and receive notifications, an observer must inherit from this
 * class.
 */
class Observer {
public:
  Observer() {}

  ~Observer() {
    disconnect_all();
  }

private:
  friend class Signal;

  void disconnect_all();

  void connect_signal(Signal *sig) {
    ScopedWrite lock(observed_signals_);
    observed_signals_.push_back(sig);
  }

  void disconnect_signal(Signal *sig) {
    ScopedWrite lock(observed_signals_);
    observed_signals_.remove(sig);
  }
  /** Signals connected.
   */
  CTList<Signal*> observed_signals_;
};

} // oscit
#endif // OSCIT_INCLUDE_OSCIT_OBSERVER_H_