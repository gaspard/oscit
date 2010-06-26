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

#ifndef OSCIT_INCLUDE_OSCIT_SIGNAL_H_
#define OSCIT_INCLUDE_OSCIT_SIGNAL_H_

#include "oscit/values.h"
#include "oscit/c_tlist.h"

#include <list>

namespace oscit {

class Signal;

/** The observer can connect member methods to slots in order
 * to receive notifications.
 */
class ObserverBz {
public:
  ObserverBz() {}

  ~ObserverBz() {
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

class Signal {
public:
  ~Signal() {
    disconnect_all();
  }

  template<class T, void(T::*Tmethod)(const Value&)>
  void connect(T *receiver) {
    SignalCallback *callback = new SignalCallback(receiver, &SignalCallback::cast_method<T, Tmethod>);

    { ScopedWrite lock(callbacks_);
      callbacks_.push_back(callback);
    }

    receiver->connect_signal(this);
  }

  void emit(const Value &val) {
    ScopedRead lock(callbacks_);
    CTList<SignalCallback*>::iterator it, end = callbacks_.end();

    for(it = callbacks_.begin(); it != end; ++it) {
      (*it)->trigger(val);
    }
  }

  void disconnect(ObserverBz *receiver) {
    disconnect_observer(receiver);
    receiver->disconnect_signal(this);
  }

private:
  friend class ObserverBz;

  typedef void (*signal_method_t)(ObserverBz *receiver, const Value &val);

  /** Object instance to trigger a member method (stores a reference to the receiver). */
  class SignalCallback {
   public:
    SignalCallback(ObserverBz *receiver, signal_method_t method)
      : receiver_(receiver), member_method_(method) {}

    void trigger(const Value &val) {
      (*member_method_)(receiver_,val);
    }

    /** Make a pointer to a member method without return values. */
    template<class T, void(T::*Tmethod)(const Value&)>
    static void cast_method(ObserverBz *receiver, const Value &val) {
      (((T*)receiver)->*Tmethod)(val);
    }

    ObserverBz     *receiver_;       /**< Object containing the method. */
    signal_method_t member_method_;  /**< Pointer on a cast of the member method. */
  };

  void disconnect_observer(ObserverBz *observer) {
    ScopedWrite lock(callbacks_);
    CTList<SignalCallback*>::iterator it, end = callbacks_.end();

    for(it = callbacks_.begin(); it != end;) {
      if ((*it)->receiver_ == observer) {
        delete *it;
        it = callbacks_.erase(it);
      } else {
        ++it;
      }
    }
  }

  void disconnect_all() {
    ScopedWrite lock(callbacks_);
    CTList<SignalCallback*>::iterator it, end = callbacks_.end();

    for(it = callbacks_.begin(); it != end;) {
      (*it)->receiver_->disconnect_signal(this);
      it = callbacks_.erase(it);
    }
  }

  CTList<SignalCallback*> callbacks_;
};


inline void ObserverBz::disconnect_all() {
  ScopedWrite lock(observed_signals_);
  CTList<Signal*>::iterator it, end = observed_signals_.end();

  for(it = observed_signals_.begin(); it != end;) {
    (*it)->disconnect_observer(this);
    it = observed_signals_.erase(it);
  }
}

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_SIGNAL_H_