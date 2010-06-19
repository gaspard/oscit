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

#ifndef OSCIT_INCLUDE_OSCIT_ROOT_H_
#define OSCIT_INCLUDE_OSCIT_ROOT_H_

#include "oscit/object.h"
#include "oscit/command.h"
#include "oscit/mutex.h"
#include "oscit/c_thash.h"
#include "oscit/c_tlist.h"
#include "oscit/object_handle.h"

namespace oscit {

/** Size of the object hash table. */
#define OBJECT_HASH_SIZE 10000

/** Size of the on register callbacks hash. */
#define CALLBACKS_ON_REGISTER_HASH_SIZE 100

#define ERROR_PATH "/.error"
#define INFO_PATH "/.info"
#define LIST_PATH "/.list"
#define LIST_WITH_TYPE_PATH "/.list_with_type"
#define REPLY_PATH "/.reply"
#define REGISTER_PATH "/.register"
#define TYPE_PATH "/.type"
#define TREE_PATH "/.tree"
#define VIEWS_PATH "/views"

class Callback;
class CallbackList;

/** Root object. You can only start new trees with root objects.

In case you intend to call elements in the object tree from different
threads, you should either manage your own mutex locks on each objects
or use a context (Mutex class or subclass). Contexts ease inter-object
communication be requiring locks/unlocks between groups of objects. Not
for every object.

*/
class Root : public Object {
 public:
  /** Class signature. */
  TYPED("Object.Root")

  Root(bool should_build_meta)
      : objects_(OBJECT_HASH_SIZE),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init(should_build_meta);
  }

  Root()
      : objects_(OBJECT_HASH_SIZE),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(const char *name)
      : Object(name),
        objects_(OBJECT_HASH_SIZE),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(const Value &type)
      : Object(type),
        objects_(OBJECT_HASH_SIZE),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(const char *name, const Value &type)
      : Object(name, type),
        objects_(OBJECT_HASH_SIZE),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(size_t hashSize)
      : objects_(hashSize),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(size_t hashSize, const char *name)
      : Object(name),
        objects_(hashSize),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(size_t hashSize, const Value &type)
      : Object(type),
        objects_(hashSize),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  Root(size_t hashSize, const char *name, const Value &type)
      : Object(name, type),
        objects_(hashSize),
        callbacks_on_register_(CALLBACKS_ON_REGISTER_HASH_SIZE) {
    init();
  }

  virtual ~Root() {
    clear();
    root_ = NULL; // avoid call to unregister_object in ~Object
  }

  /** Clear (remove all objects).
   * Thread safe.
   */
  void clear();

  /** Start listening for incomming messages from the given command.
   * Thread safe.
   */
  template<class T>
  T * adopt_command(T *command, bool start = true) {
    if (command_for_protocol(command->protocol()) != NULL) {
      // we already have a command for this protocol. Do not adopt.
      delete command;
      return NULL;
    }
    command->set_root(this);

    { ScopedWrite lock(commands_);
      commands_.push_back(command);
    }

    if (start) {
      command->start_command();
    }

    return command;
  }

  /** Forget about a command.
   * TODO: clarify who should use this.
   * Thread safe.
   */
  void unregister_command(Command *command) {
    ScopedWrite lock(commands_);
    commands_.remove(command);
  }

  /** Return (create if necessary) the "/views" container.
   * Thread safe.
   */
  bool get_views_path(ObjectHandle *handle) {
    if (get_object_at(VIEWS_PATH, handle)) {
      return true;
    } else {
      Object *object = adopt(new Object(Url(VIEWS_PATH).name(), NoIO("All objects under this path are views.")));
      if (object) {
        handle->hold_fast(object);
        return true;
      } else {
        return false;
      }
    }
  }

  /** Expose all views in folder and enable creation/deletion of views.
   */
  virtual bool expose_views(const std::string &path, Value *error);

  /** Expose all views in folder and enable creation/deletion of views.
   */
  bool expose_views(const char *path, Value *error) {
    return expose_views(std::string(path), error);
  }


  /** Trigger the object located at the given path, passing nil as parameter.
   * Thread safe.
   */
  const Value call(const char *path) {
    return call(Url(Location::NO_IP, Location::NO_PORT, path), gNilValue);
  }

  /** Trigger the object located at the given path, passing nil as parameter.
   * Thread safe.
   */
  const Value call(const std::string &path) {
    return call(Url(Location::NO_IP, Location::NO_PORT, path), gNilValue);
  }

  /** Trigger the object located at the given path with the given parameters.
   * Thread safe.
   */
  const Value call(const char *path, const Value &val) {
    return call(Url(Location::NO_IP, Location::NO_PORT, path), val);
  }

  /** Trigger the object located at the given path with the given parameters.
   * Thread safe.
   */
  const Value call(const std::string &path, const Value &val) {
    return call(Url(Location::NO_IP, Location::NO_PORT, path), val);
  }

  /** Trigger the object in the local tree located at the given url. At this point, the
   * url's location contains the origin of the call.
   * Thread safe.
   */
  const Value call(const Url &url, const Value &val) {
    ObjectHandle handle;
    Value error;

    if (!find_or_build_object_at(url.path(), &error, &handle)) {
      return error;
    }

    return call(handle, val, &url.location());
  }

  /** Call an object's trigger method.
   */
  inline const Value call(ObjectHandle &target, const Value &val, const Location *origin) {
    if (val.is_empty()) return call(target, gNilValue, origin);
    if (target->can_receive(val)) {
      return target->trigger(val);
    } else {
      return ErrorValue(BAD_REQUEST_ERROR, std::string("'").append(
        target->url()).append("' (").append(
        target->type().last().str()).append(")."));
    }
  }

  /** Send a message to a given location.
   * (Thread safe).
   */
  void send(const Location &remote, const char *path, const Value &val) {
    // FIXME: CommandHandle
    Command *cmd = command_for_protocol(remote.protocol());
    if (cmd) {
      cmd->send(remote, path, val);
    } else {
      std::cerr << "Cannot send '" << path << "(" << val << ")" << "' to " << remote << " no command for protocol '" << remote.protocol() << "'.\n";
    }
  }

  /** Send value to given url (can be local or remote). You should use 'call' for local urls (faster).
   * FIXME: this needs refactoring... we should not have a 'send' finishing in a 'call' !
   * FIXME: maybe rename this to 'call_remote' ?
   * (Thread safe).
   */
  const Value send(const Url &url, const Value &val) {
    ObjectHandle handle;
    Value error;
    if (!get_object_at(url, &error, &handle)) {
      return error;
    }

    return call(handle, val, &url.location());
  }

  /** Find any object (local or remoate).
   * Thread safe.
   */
  bool get_object_at(const Url &url, Value *error, ObjectHandle *handle) {
    if (url.path() == "") {
      // bad url
      error->set(BAD_REQUEST_ERROR, std::string("Could not parse url '").append(url.str()).append("'."));
      return false;
    } else if (url.protocol() == "") {
      // local object
      return find_or_build_object_at(url.str(), error, handle);
    } else {
      // remote
      // find command for given protocol
      Command * cmd = command_for_protocol(url.protocol());
      if (cmd) {
        // what is this ? Why do we need 'remote_object' and not just ObjectProxy ?
        return cmd->remote_object(url, error, handle);
      } else {
        // unknown url protocol
        error->set(BAD_REQUEST_ERROR, std::string("No command to handle '").append(url.protocol()).append("' protocol."));
        return false;
      }
    }
  }

  /** Add a callback on object registration.
   * Thread safe.
   */
  void adopt_callback_on_register(const std::string &url, Callback *callback);

  /** Trigger and remove all callbacks for a specific url registration.
   * Thread safe.
   * TODO: make private
   */
  void trigger_and_clear_on_register_callbacks(const std::string &url);

  /** Remove all on register callbacks.
   * Thread safe.
   * TODO: make private
   */
  void clear_on_register_callbacks();

  /** Notification of name/parent change from an object. This method
   * keeps the objects dictionary in sync.
   * Thread safe.
   */
  void register_object(Object *obj);

  /** Unregister an object from tree (forget about it).
   * Thread safe.
   */
  void unregister_object(Object *obj);

  /** Find a pointer to an Object from its path. Return false if the object is not found.
   * Thread safe.
   */
  bool get_object_at(const std::string &path, ObjectHandle *handle) {
    ScopedRead lock(objects_);
    Object *res;
    if (objects_.get(std::string(path), &res)) {
     handle->hold_fast(res);
     return true;
    } else {
     return false;
    }
  }

  /** Find a pointer to an Object from its path. Return false if the object is not found.
   * Thread safe.
   */
  bool get_object_at(const char *path, ObjectHandle *handle) {
    ScopedRead lock(objects_);
    Object *res;
    if (objects_.get(std::string(path), &res)) {
      handle->hold_fast(res);
      return true;
    } else {
      return false;
    }
  }

  /** Find the object at the given path. Before raising a 404 error, we try to find a 'not_found'
   * handler that could build the resource.
   * Thread safe.
   */
  bool find_or_build_object_at(const std::string &path, Value *error, ObjectHandle *handle) {
    bool ok = do_find_or_build_object_at(path, error, handle);

    if (!ok && (error->is_empty() || (error->is_error() && error->error_code() == NOT_FOUND_ERROR))) {
      error->set(NOT_FOUND_ERROR, path);
    }

    return ok;
  }

  /** Find the object at the given path. Before raising a 404 error, we try to find a 'not_found'
   * handler that could build the resource.
   * Thread safe.
   */
  inline bool find_or_build_object_at(const char *path, Value *error, ObjectHandle *handle) {
    return find_or_build_object_at(std::string(path), error, handle);
  }

  /* ======================= META METHODS HELPERS ===================== */

  /** Send a reply to all commands so they pass it further to their observers.
   * Thread safe.
   * TODO: make private
   */
  void notify_observers(const char *path, const Value &val) {
    ScopedRead lock(commands_);
    std::list<Command*>::iterator it;
    std::list<Command*>::iterator end = commands_.end();
    for (it = commands_.begin(); it != end; ++it) {
      (*it)->notify_observers(path, val);
    }
  }

  /** Find a command handling a given protocol.
   * TODO: make private ?
   * (Thread safe).
   * TODO: use a CommandHandle with reference counting.
   */
  Command *command_for_protocol(const std::string& protocol) {
    ScopedRead lock(commands_);
    std::list<Command*>::iterator it;
    std::list<Command*>::iterator end = commands_.end();
    for (it = commands_.begin(); it != end; ++it) {
      if ((*it)->protocol() == protocol) {
        return *it;
      }
    }
    return NULL;
  }

 protected:
  /** Hash to find any object in the tree from its path.
   */
  CTHash<std::string, Object*> objects_;

 private:
  bool do_find_or_build_object_at(const std::string &path, Value *error, ObjectHandle *handle) {
    if (get_object_at(path, handle)) {
      return true;
    } else {
      // ask parents to build
      size_t pos = path.rfind("/");
      if (pos != std::string::npos) {
        // call 'build_child' in parent.
        ObjectHandle parent;
        if (do_find_or_build_object_at(path.substr(0, pos), error, &parent)) {
          if (get_object_at(path, handle)) {
            // parent has automatically created the child object
            return true;
          } else {
            // ask parent to create child
            return parent->build_child(path.substr(pos+1), gNilValue, error, handle);
          }
        } else {
          // could not build parent
          return false;
        }
      } else {
        // no parent
        return false;
      }
    }
  }

  void init(bool should_build_meta = true);

  /** Listening commands (only one allowed per protocol).
   */
  CTList<Command *> commands_;

  /** List of callbacks to trigger on object registration.
   */
  CTHash<std::string, CallbackList*> callbacks_on_register_;

};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_ROOT_H_
