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

#include "oscit/root_proxy.h"

#include <string>
#include <iostream>

#include "oscit/command.h"
#include "oscit/object_proxy.h"
#include "oscit/proxy_factory.h"

namespace oscit {

void RootProxy::set_command(Command *command) {
  if (command_) command_->unregister_proxy(this);
  command_ = command;
  if (command) {
    command->register_proxy(this);
    command->send(remote_location_, REGISTER_PATH, gNilValue);
    sync_children();
  }
}

void RootProxy::set_proxy_factory(ProxyFactory *factory) {
  if (proxy_factory_) proxy_factory_->unregister_proxy(this);
  proxy_factory_ = factory;
  if (proxy_factory_) proxy_factory_->register_proxy(this);
}


void RootProxy::build_children_from_attributes(Object *parent, const Value &list) {
  if (!list.is_list() || list.size() % 2 != 0) {
    std::cerr << "Cannot handle " << LIST_WITH_ATTRIBUTES_PATH << " reply: invalid argument: " << list << "\n";
    return;
  }

  if (!proxy_factory_) {
    std::cerr << "Cannot handle " << LIST_WITH_ATTRIBUTES_PATH << " reply: no ProxyFactory !\n";
    return;
  }

  int list_count = list.size() / 2;
  Value name_with_type;
  ObjectProxy *object_proxy;
  bool has_children;


  for(int i=0; i < list_count; ++i) {
    Value name_value = list[2 * i];
    Value attributes = list[2 * i + 1];
    if (!name_value.is_string() || !attributes.is_hash()) {
      std::cerr << "Invalid argument in " << LIST_WITH_ATTRIBUTES_PATH << " reply: " << name_value << " => " << attributes << "\n";
    } else {
      std::string name = name_value.str();
      ObjectHandle object;

      if (name.size() > 1 && name.at(name.length()-1) == '/') {
        has_children = true;
        name = name.substr(0, name.size() - 1);
      } else {
        has_children = false;
      }

      if (!parent->get_child(name, &object)) {
        // child does not exist yet, first ask parent if it can build it
        Value error;
        if (parent->build_child(name, attributes, &error, &object)) {
          object_proxy = object.type_cast<ObjectProxy>();
          if (!object_proxy) {
            std::cerr << parent->url() << " has built an invalid object through 'build_child'. Should be an ObjectProxy, found a " << object->class_name() << "\n";
          } else {
            object_proxy->set_need_sync(has_children);
          }
        } else if (error.is_error()) {
          std::cerr << parent->url() << "Returned an error while trying to build " << name << ": " << error << "\n";
        } else {
          object_proxy = proxy_factory_->build_object_proxy(parent, name, attributes);
          if (object_proxy) {
            parent->adopt(object_proxy);
            object_proxy->set_need_sync(has_children);
          }
        }
      }
    }
  }
}

bool RootProxy::build_child(const std::string &name, const Value &attrs, Value *error, ObjectHandle *handle) {
  if (!proxy_factory_) {
    std::cerr << "Cannot build child /" << name << " : no ProxyFactory !\n";
    return false;
  }

  Object *object = proxy_factory_->build_object_proxy(this, name, attrs);

  if (object) {
    adopt(object);
    handle->hold(object);
    return true;
  } else {
    return false;
  }
}

void RootProxy::handle_reply(const std::string &path, const Value &val) {
  if (path == LIST_WITH_ATTRIBUTES_PATH) {
    // s[sHsHsH...]
    // "parent path", ["child name", { attributes }, "child name", { attributes }, ...]
    if (val.size() < 2 || !val[0].is_string() || !val[1].is_list()) {
      std::cerr << "Invalid argument in " << LIST_WITH_ATTRIBUTES_PATH << " reply: " << val << "\n";
      return;
    }

    ObjectHandle handle;
    if (!get_object_at(val[0].str(), &handle)) {
      std::cerr << "Invalid parent path " << val[0].str() << " in " << LIST_WITH_ATTRIBUTES_PATH << " reply: unknown path.\n";
      return;
    }

    build_children_from_attributes(handle.ptr(), val[1]);
  } else if (path == ATTRS_PATH) {
    if (val.size() < 2 || !val[0].is_string()) {
      std::cerr << "Invalid argument in " << ATTRS_PATH << " reply: " << val << "\n";
      return;
    }

    Value error;
    ObjectProxy *object_proxy = NULL;
    ObjectHandle handle;
    if (find_or_build_object_at(val[0].str(), &error, &handle)) {
      object_proxy = handle.type_cast<ObjectProxy>();
    }

    if (object_proxy) {
      object_proxy->set_attrs(val[1]);
    }
  } else {
    // Find target
    ObjectHandle object;

    if (get_object_at(path, &object) && object->can_receive(val)) {
      ObjectProxy *object_proxy = object.type_cast<ObjectProxy>();
      // TODO: Should we use 'safe' here ? (mutex)
      if (object_proxy) object_proxy->handle_value_change(val);
    } else {
      std::cerr << "could not route '" << path << "' reply to '" << val << "'\n";
    }
  }
}

} // oscit
