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

#include "oscit/root.h"

// meta methods
#include "oscit/info_meta_method.h"
#include "oscit/type_meta_method.h"
#include "oscit/error_meta_method.h"
#include "oscit/list_meta_method.h"
#include "oscit/list_with_type_meta_method.h"
#include "oscit/tree_meta_method.h"

#include "oscit/callback_list.h"
#include "oscit/file.h"
#include "oscit/hash_file_method.h"

namespace oscit {

void Root::init(bool should_build_meta) {
  url_ = "";
  set_root(this);

  if (should_build_meta) {
    // build meta methods
    adopt(new ErrorMetaMethod(Url(ERROR_PATH).name()));
    adopt(new InfoMetaMethod(Url(INFO_PATH).name()));
    adopt(new ListMetaMethod(Url(LIST_PATH).name()));
    adopt(new ListWithTypeMetaMethod(Url(LIST_WITH_TYPE_PATH).name()));
    adopt(new TypeMetaMethod(Url(TYPE_PATH).name()));
    adopt(new TreeMetaMethod(Url(TREE_PATH).name()));
  }
}

void Root::clear() {
  { ScopedWrite lock(commands_);
    while (!commands_.empty()) {
      Command *command = commands_.front();
      command->kill();
      command->set_root(NULL); // avoid call to unregister_command in ~Command

      delete command;
      commands_.pop_front();
    }
  }

  clear_on_register_callbacks();

  this->Object::clear();
}

void Root::clear_on_register_callbacks() {
  ScopedWrite lock(callbacks_on_register_);

  std::list<std::string>::iterator it, end = callbacks_on_register_.end();
  CallbackList *callbacks;

  for (it = callbacks_on_register_.begin(); it != end; ++it) {
    if (callbacks_on_register_.get(*it, &callbacks)) {
      delete callbacks;
    }
  }
}

void Root::adopt_callback_on_register(const std::string &url, Callback *callback) {
  ScopedWrite lock(callbacks_on_register_);

  CallbackList *callbacks;
  if (!callbacks_on_register_.get(url, &callbacks)) {
    callbacks = new CallbackList(this);
    callbacks_on_register_.set(url, callbacks);
  }
  callbacks->adopt_callback(callback);
}

void Root::trigger_and_clear_on_register_callbacks(const std::string &url) {
  ScopedWrite lock(callbacks_on_register_);
  CallbackList *callbacks;

  if (callbacks_on_register_.get(url, &callbacks)) {
    callbacks->trigger_all();
    delete callbacks;
    callbacks_on_register_.remove(url);
  }
}

void Root::register_object(Object *obj) {
  ObjectHandle hold(obj);

  { ScopedWrite lock(objects_);
    // add object to objects dictionary
    objects_.set(obj->url(), obj);
  }

  trigger_and_clear_on_register_callbacks(obj->url());

  if (!Url::is_meta(obj->url())) {
    Value type(obj->url());
    type.push_back(obj->type_with_current_value());
    Value reply(TYPE_PATH);
    reply.push_back(type);
    notify_observers(REPLY_PATH, reply);
  }
}

void Root::unregister_object(Object *obj) {
  ScopedWrite lock(objects_);
  objects_.remove_element(obj);
}

bool Root::expose_views(const std::string &path, Value *error) {
  // create '/views' url
  ObjectHandle views;
  if (!get_views_path(&views)) {
    error->set(INTERNAL_SERVER_ERROR, "Could not create views path");
    return false;
  }

  Value view_list = File::list(path, ".json");
  HashFileMethod *view;
  Value test;
  ListValue errors;

  bool all_ok = true;
  for(size_t i = 0; i < view_list.size(); ++i) {
    view = views->adopt(new HashFileMethod(view_list[i].str().substr(0, view_list[i].str().length() - 5), File::join(path, view_list[i].str()), "view"));
    test = view->trigger(gNilValue);
    if (test.is_error()) {
      errors.push_back(test);
      all_ok = false;
    }
  }

  if (all_ok) {
    return true;
  } else {
    error->set(UNKNOWN_ERROR, errors.to_json());
    return false;
  }
}


} // oscit


