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

#include "oscit/object_proxy.h"

#include "oscit/root.h"
#include "oscit/root_proxy.h"
#include "oscit/proxy_factory.h"

namespace oscit {

void ObjectProxy::adopted() {
  root_proxy_ = TYPE_CAST(RootProxy, root_);
  if (root_proxy_ && type().is_nil()) {
    // try to find type
    root_proxy_->send_to_remote(ATTRS_PATH, Value(url()));
  } else if (value_.is_empty()) {
    // only get initial value if type is already there (or we won't receive the value).
    set_value(gNilValue);
  }
}

void ObjectProxy::set_value(const Value &val) {
  assert(root_proxy_);

  // std::cout << url() << ": set_value(" << val << ")\n";
  if (can_receive(val)) {
    time_t now = time_ref_.elapsed();
    // only send if value type is correct
    if (latency_ < 0) {
      if (wait_until_ >= 0) {
        // waiting for first call reply: wait
        // std::cout << url() << ": waiting for first call reply: wait\n";
      } else {
        // first call
        root_proxy_->send_to_remote(url().c_str(), val);
        wait_until_ = now;
      }
    } else if (now < wait_until_) {
      // do not send
      // std::cout << url() << ": to send\n";
      to_send_ = val;
    } else {
      root_proxy_->send_to_remote(url().c_str(), val);
      wait_until_ = now + (latency_ * 1.5);
    }
  }
}

void ObjectProxy::handle_value_change(const Value &val) {
  // TODO: root should check type
  value_ = val;
  if (latency_ < 0 && wait_until_ >= 0) {
    // latency not set but we have already sent a value out

    // we guess we are receiving from our own send...
    // TODO: this can lead to latency_ being too short if we receive another
    // notification, but it does not matter
    latency_ = time_ref_.elapsed() - wait_until_;
    wait_until_ = 0;
  } else {
    if (wait_until_) {
      // (gently) update latency
      latency_ = latency_ + (time_ref_.elapsed() - wait_until_) / 4;
    }

    if (!to_send_.is_empty()) {
      // send now
      root_proxy_->send_to_remote(url().c_str(), to_send_);
      to_send_.set_empty();
      wait_until_ = time_ref_.elapsed() + (latency_ * 1.5);
    } else {
      wait_until_ = 0;
    }
  }
  value_changed();
}

void ObjectProxy::set_attrs(const Value &new_attrs) {
  if (new_attrs != attributes_) {
    bool need_initial_value = !attributes_.is_hash();
    attributes_ = new_attrs;

    sync_type_id();
    attrs_changed();

    if (need_initial_value && root_proxy_) {
      // We check for root_proxy_ because our object might have attrs set before being adopted.
      // get initial value
      set_value(gNilValue);
    }
  }
}

bool ObjectProxy::build_child(const std::string &name, const Value &attrs, Value *error, ObjectHandle *handle) {
  if (!root_proxy_ || !root_proxy_->proxy_factory()) {
    std::cerr << "Cannot build child /" << name << " : no RootProxy or no ProxyFactory !\n";
    return false;
  }
  Object *obj = adopt(root_proxy_->proxy_factory()->build_object_proxy(this, name, attrs));
  if (obj) {
    handle->hold(obj);
    return true;
  } else {
    return false;
  }
}

} // oscit