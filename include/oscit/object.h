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


#ifndef OSCIT_INCLUDE_OSCIT_OBJECT_H_
#define OSCIT_INCLUDE_OSCIT_OBJECT_H_

#include <stdlib.h> // atoi
#include <list>
#include <string>

#include "oscit/constants.h"
#include "oscit/typed.h"
#include "oscit/observer.h"
#include "oscit/signal.h"
#include "oscit/values.h"
#include "oscit/thash.h"
#include "oscit/mutex.h"
#include "oscit/location.h"
#include "oscit/c_reference_counted.h"
#include "oscit/c_tvector.h"
#include "oscit/c_thash.h"

namespace oscit {

/** Used to append the "-1" or "-2" to osc methods.
 */
#define OSC_NEXT_NAME_BUFFER_SIZE 20

class Root;
class Alias;
class ObjectProxy;
class ObjectHandle;

class Object : public Typed, public Observer, public CReferenceCounted {
 public:
  /** Class signature. */
  TYPED("Object")

  explicit Object() : root_(NULL), parent_(NULL), children_(20), context_(NULL), keep_last_(false),
    attributes_(Attribute::default_io()) {
    sync_type_id();
    name_ = "";
    url_  = name_;
  }

  explicit Object(const char *name) : root_(NULL), parent_(NULL),
    children_(20), name_(name), url_(name), context_(NULL), keep_last_(false),
    attributes_(Attribute::default_io()) {
    sync_type_id();
  }

  explicit Object(const std::string &name) : root_(NULL), parent_(NULL),
    children_(20), name_(name), url_(name), context_(NULL), keep_last_(false),
    attributes_(Attribute::default_io()) {
    sync_type_id();
  }

  explicit Object(const Value &attrs) : root_(NULL), parent_(NULL),
    children_(20), context_(NULL), keep_last_(false), attributes_(attrs) {
    sync_type_id();
    name_ = "";
    url_  = name_;
  }

  Object(const char *name, const Value &attrs, bool keep_last = false) : root_(NULL), parent_(NULL),
    children_(20), name_(name), url_(name), context_(NULL), keep_last_(keep_last),
    attributes_(attrs) {
    sync_type_id();
  }

  Object(const std::string &name, const Value &attrs, bool keep_last = false) : root_(NULL),
    parent_(NULL), children_(20), name_(name), url_(name), context_(NULL),
    keep_last_(keep_last), attributes_(attrs) {
    sync_type_id();
  }

  Object(Object *parent, const char *name) : root_(NULL), parent_(NULL),
    children_(20), name_(name), context_(NULL), keep_last_(false),
    attributes_(Attribute::default_io()) {
    sync_type_id();
    parent->adopt(this);
  }

  Object(Object *parent, const char *name, const Value &attrs) : root_(NULL),
    parent_(NULL), children_(20), name_(name), context_(NULL), keep_last_(false),
    attributes_(attrs) {
    sync_type_id();
    parent->adopt(this);
  }

  Object(Object *parent, const std::string &name, const Value &attrs) :
    root_(NULL), parent_(NULL), children_(20), name_(name), context_(NULL),
    keep_last_(false), attributes_(attrs) {
    sync_type_id();
    parent->adopt(this);
  }

  virtual ~Object();

  /** Clear all children (delete).
   * TODO: make thread safe
   */
  void clear();

  /** This is the operation executed when the object is called.
   * In order to benefit from return value optimization and avoid too many copy
   * you have to use Value v = xxx.trigger(val).
   * @todo why is the return value const ?
   */
  virtual const Value trigger(const Value &val) {
    return gNilValue;
  }

  /** Dynamically build a child from the given name. This method is called whenever
   * a sub-node or branch is not found and this is the last found object along
   * the path.
   */
  virtual bool build_child(const std::string &name, const Value &attrs, Value *error, ObjectHandle *handle) {
    return false;
  }

  /** Lock resource (used by commands). */
  inline void lock() {
    if (context_) context_->lock();
  }

  /** Unlock resource (used by commands). */
  inline void unlock() {
    if (context_) context_->unlock();
  }

  /** Return the object's unique url. */
  inline const std::string &url() const {
    return url_;
  }

  /** Set name from string. If the name is not unique in the parent's scope,
   *  the name is changed as "name-1", "name-2", etc.
   */
  void set_name(const char* name) { set_name(std::string(name)); }


  /** Set name from string. If the name is not unique in the parent's scope,
   *  the name is changed as "name-1", "name-2", etc.
   */
  void set_name(const std::string &name) {
    if (name == "") return;
    name_ = name;  // FIXME: gsub(/[^a-zA-Z\-0-9_],"")
    set_parent(parent_);  // reset parent so name is verified in parent
    moved();
  }

  void set_keep_last(bool should_keep_last) {
    if (should_keep_last != keep_last_) {
      keep_last_ = should_keep_last;
      set_parent(parent_); // reset
    }
  }
  /** Return name of object. */
  inline const std::string name() const {
    return name_;
  }

  /** ========================== REPLIES TO META METHODS ======================
   * The replies to meta methods are implemented as virtuals so that objects
   * that inherit from osc::Object just need to overwrite these in order to
   * return more meaningful information / content.
   */

  /** List sub-nodes.
   * This method is used as a reply to the /.list meta method.
   * The format of the reply is a list of names with the type:
   * [name, name, ...].
   */
  const Value list() const;

  /** Shortcut to call multiple methods on an object.
   * @param val Using "obj.set(tempo:45 rubato:1.5)" is equivalent to calling
   *            "obj.tempo(45)" and "obj.rubato(1.5)".
   * @return    a hash with the result for each call.
   */
  virtual const Value set(const Value &hash);

  /** Shortcut to call multiple methods on an object.
   * @param val Using "obj.set(tempo:45 rubato:1.5)" is equivalent to calling
   *            "obj.tempo(45)" and "obj.rubato(1.5)".
   * @return true on success, false if any call failed.
   */
  bool set_all_ok(const Value &val) {
    Value result = set(val);
    return !result.contains_error();
  }

  /** This is the prefered way to insert new objects in the tree since it clearly
   * highlights ownership in the parent.
   * TODO: make sure a parent is not adopted by it's child.
   * TODO: if we want to make this thread safe, we need a handle.
   */
  template<class T>
  T *adopt(T *object) {
    object->set_parent(this);
    return object;
  }

  /** Return a hash representing the current object. If the current object is a container
   * (no_io type), the default behavior is to build a hash by sending 'to_hash'
   * on the children objects and merge the current attributes. If the object is a method
   * this is an alias for 'trigger'.
   *
   * This method is not const because the objects might need to trigger to get their
   * current value.
   */
  virtual const Value to_hash();

  /** List sub-nodes with their current attributes.
   * This method is used as a reply to the /.list_att meta method.
   * The format of the reply is a list of names with the type:
   * [name, { attributes }, name, { attributes }, etc].
   */
  const Value list_with_attributes() const;

  /** List full tree under this node.
   *  @param base_length is the length of the url for the initial call
   *                     (removed from results).
   *  @param tree returned value.
   */
  void tree(size_t base_length, Value *tree) const;

  /** Human readable information method.
   *  Called as a response to "/.info '/this/url'".
   */
  const Value info() const {
    Value info(attributes_[Attribute::INFO]);
    return info.is_string() ? info : Value("Container.");
  }

  /** Type information on node.
   */
  const Value type() const {
    return attributes_[Attribute::TYPE];
  }

  /** Object attributes hash.
   */
  const Value &attributes() const {
    return attributes_;
  }

  /** Object attributes hash.
   * Changing any attribute but TYPE is ok. Changing TYPE will generate problems (at least for now).
   */
  Value &attributes() {
    return attributes_;
  }
  /** Set meta type (signature, range, units). The type should be immutable.
   *  this method is not a good idea.
   */
  // bool set_type(const Value &attrs) {
  //   if (type.type_id() != type_.type_id()) {
  //     return false;
  //   } else {
  //     type_ = type;
  //     sync_type_id();
  //     return true;
  //   }
  // }

  /** Return the list of children as a hash.
   * FIXME: if this is a const how does the user of this method RW lock ?
   */
  const CTHash<std::string, Object *> &children() const {
    return children_;
  }

  /** Return first child.
   */
  bool first_child(ObjectHandle *handle) {
    return get_child(0, handle);
  }

  /** Tries to find a child named 'name'.
   */
  bool get_child(const std::string &name, ObjectHandle *handle);

  /** Tries to find the child at a specific index. Returns
   * false on failure.
   */
  bool get_child(size_t index, ObjectHandle *handle);

  /** Return the number of children objects.
   */
  size_t children_count() {
    ScopedRead lock(children_vector_);
    return children_vector_.size();
  }


  Object *parent() {
    return parent_;
  }

  /** This method is called whenever the parent changes. It can be
   * used to finish initializing the object.
   */
  virtual void adopted() {}

  /** Set object's new parent.
   */
  void set_parent(Object *parent);

  /** Parent changed, set new context. */
  virtual void set_context(Mutex *context) { context_ = context; }

  /** Return the type tag signature id (uint) of the trigger method of this
   * object (what it wants to receive as arguments).
   *
   * TODO: rename, it's confusing with type().type_id(). Use accept_type_id() instead ?
   */
  inline TypeTagID type_id() {
    return type_id_;
  }

  bool accept_any_type() {
    return type_id_ == ANY_TYPE_TAG_ID;
  }

  /** This method returns true if the current object can receive the given Value.
   * The rule is as follows:
   *
   * # NoIO do not receive anything
   * # AnyIO accept anything
   * # BangValue and NilValues are received by all
   * # All IO types (except NoIO) accept the Value corresponding to their type
   */
  inline bool can_receive(const Value &val) {
    return can_receive(val.type_id());
  }

  /** This method returns true if the current object can receive the given TypeId.
   * The rules are the same for can_receive(const Value &val).
   */
  inline bool can_receive(uint type_id) {
    if (type_id_ == NO_TYPE_TAG_ID) return false;
    if (type_id == type_id_ || accept_any_type()) {
      return true;
    } else if (type_id == NIL_TYPE_TAG_ID || type_id == TRUE_TYPE_TAG_ID) { // nil or Bang
      return true;
    // Bad idea (do we need this ?)
    // } else if (val.starts_with_type_tags(type_signature())) {
    //   // first match
    //   return true;
    } else {
      return false;
    }
  }
  /** Signal to receive notifications on object destruction.
   */
  Signal &on_delete() {
    return on_delete_;
  }

 protected:

  /** Set object's new root.
  */
  void set_root(Root *root);

  /** Child sends a notification to the parent when it's name changes so that
   *  the parent/root keep their url hash in sync.
   */
  void register_child(Object *child);

  /** Free the child from the list of children.
   */
  void unregister_child(Object *child);

  /** Update cached url, notify root_ of the position change.
   */
  void moved();

  /** Add '-1', '-2', ... at the end of the current name. bob --> bob-1
   */
  void find_next_name() {
    size_t pos = name_.find('-');
    char buffer[OSC_NEXT_NAME_BUFFER_SIZE];
    if (pos == std::string::npos) {
      name_.append("-1");
    } else {
      std::string baseName = name_.substr(0, pos - 1);
      int i = atoi(name_.substr(pos + 1, std::string::npos).c_str());
      snprintf(buffer, OSC_NEXT_NAME_BUFFER_SIZE, "%i", i+1);
      name_ = baseName.append(buffer);
    }
  }

 private:
  friend class ObjectProxy;
  friend class Alias;

  /** Keep type_id_ in sync with type_.
   */
  void sync_type_id() {
    Value signature = attributes_[Attribute::TYPE][Attribute::SIGNATURE];
    if (signature.is_string()) {
      type_id_ = hashId(signature.str());
    } else {
      // invalid
      type_id_ = NO_TYPE_TAG_ID;
    }
  }

 protected:

  /** Root object used to access application methods/
   */
  Root *root_;

  /** Pointer to parent object.
   * Can be NULL if the object does not have a parent.
   */
  Object *parent_;

  /** Hash with pointers to children objects or methods.
   * The children objects unregister themselves when they die or change
   * their parent by calling 'unregister_child'.
   */
  CTHash<std::string, Object *> children_;

  /** List of children as a vector for faster enumeration type of access.
   * FIXME: remove, bad optimization.
   */
  CTVector<Object*> children_vector_;

  /** Unique name in parent's context.
   */
  std::string name_;

  /** Absolute path to object (cached).
   * FIXME: clarify usage and RW lock need. This is not very DRY with name_.
   */
  std::string url_;

  /** Mutex to make sure only one thread is using a given context at a time.
   * FIXME: remove.
   */
  Mutex *context_;

  /** Flag to force this object at the end of the children list.
   * If more then one child has this setting, they are kept in insertion order at
   * the end.
   */
  bool keep_last_;


  /** Signal to notify destruction.
   * Thread safe.
   */
  Signal on_delete_;

  /** Value that holds all attributes including type information on the 'trigger' method of this
   * object.
   * If the attributes does not contain a "type" key, this means that the object is not callable.
   * This can mean that the object is either a container or a widget.
   *
   */
  Value attributes_;

private:

  /** Identifier for the type of the values the element can receive.
   * The value is a hash of the type tag list (multiple osc type tags) such
   * as "f" (single Real) or "fss" (Real, String, String).
   * This is cached from the first element in type_.
   * FIXME: clarify usage and RW lock need.
   */
  TypeTagID type_id_;
};

}  // namespace osc

#endif  // OSCIT_INCLUDE_OSCIT_OBJECT_H_
