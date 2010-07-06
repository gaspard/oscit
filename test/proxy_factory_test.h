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

#include "test_helper.h"
#include "oscit/root.h"
#include "oscit/root_proxy.h"
#include "oscit/object_proxy.h"
#include "oscit/proxy_factory.h"
#include "mock/dummy_object.h"
#include "mock/observer_logger.h"
#include "mock/object_proxy_logger.h"

/** Integration tests for the ProxyFactory pattern (local proxy kept in sync with remote values).
 */

class PFTLogger : public ObserverLogger {
public:
  PFTLogger(const char *id, Logger *oss) : ObserverLogger(id, oss) {}

  void foo(const Value &val) {
    *oss_ << "[" << id_ << "#foo " << val << "]";
  }

  void bar(const Value &val) {
    *oss_ << "[" << id_ << "#bar " << val << "]";
  }

  void dummy_view(const Value &val) {
    *oss_ << "[" << id_ << "#dummy_view " << val << "]";
  }
};

/* Widgets */

class UpdateDummyView : public ObjectProxy {
public:
  /** Class signature. */
  TYPED("Object.ObjectProxy.UpdateDummyView")

  UpdateDummyView(const std::string &name, const Value &attrs)
      : ObjectProxy(name, attrs) {}
};

class DummyView : public ObjectProxyLogger {
public:
  /** Class signature. */
  TYPED("Object.ObjectProxy.ObjectProxyLogger.DummyView")

  DummyView(const std::string &name, const Value &attrs)
      : ObjectProxyLogger(name, attrs) {}

  virtual void adopted() {
    ObjectProxyLogger::adopted();
    // force immediate sync
    sync_children();
  }

  virtual bool build_child(const std::string &name, const Value &attrs, Value *error, ObjectHandle *handle) {
    if (name == "update") {
      Object *obj = adopt(new UpdateDummyView(name, attrs));
      if (obj) {
        handle->hold(obj);
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
};

class MyRootProxy : public RootProxy {
public:
  MyRootProxy(const Location &location) : RootProxy(location) {}

};

class MyObjectProxy : public ObjectProxyLogger {
public:
  MyObjectProxy(const std::string &name, const Value &attrs) : ObjectProxyLogger(name, attrs) {}
};


class MyProxyFactory : public ProxyFactory {
public:
  MyProxyFactory() : built_(10) {}

  virtual ObjectProxy *build_object_proxy(Object *parent, const std::string &name, const Value &attrs) {
    if (is_meta_method(name)) return NULL;

    ObjectProxyLogger *object_proxy;

    if (name == "dummy_view") {
      // build a dummy 'view' proxy to test if it builds the 'update' method
      object_proxy = new DummyView(name, attrs);
    } else {
      object_proxy = new MyObjectProxy(name, attrs);
    }

    built_.set(name, object_proxy);
    return object_proxy;
  }

  virtual RootProxy *build_root_proxy(const Location &location) {
    return new MyRootProxy(location);
  }

  ObjectProxyLogger *find_by_name(const char *name) {
    ObjectProxyLogger *object_proxy = NULL;
    built_.get(std::string(name), &object_proxy);
    return object_proxy;
  }

private:
  THash<std::string, ObjectProxyLogger*> built_;
};

/*
local + OscCommand = local
remote + OscCommand = remote
proxy = proxy of remote remote in local context
*/

class ProxyFactoryTest : public TestHelper
{
 public:
  enum {
    REMOTE_PORT = 7019,
    LOCAL_PORT = 7018
  };

  void test_build_and_sync( void ) {
    // prepare remote root
    Root remote;
    remote.adopt_command(new OscCommand(REMOTE_PORT));
    Object *monitor = remote.adopt(new Object("monitor"));
    monitor->adopt(new DummyObject("mode", "rgb", Attribute::select_io("rgb, yuv", "This is a menu.")));
    monitor->adopt(new DummyObject("tint", 45.0, Attribute::range_io(1, 127, "This is a slider from 1 to 127.")));

    // prepare local root
    Root local;
    OscCommand *cmd = local.adopt_command(new OscCommand(LOCAL_PORT));

    // factory
    MyProxyFactory factory;

    // build proxy
    MyRootProxy *proxy = (MyRootProxy*) factory.build_and_init_root_proxy(Location("oscit", Location::LOOPBACK, REMOTE_PORT));

    // sync first level (root children)
    cmd->adopt_proxy(proxy); // sync ----> cmd ----> remote -----> cmd -----> "/.reply" ----> proxy
    millisleep(100);
    Value res = proxy->list();
    assert_equal("[\"monitor\"]", res.to_json());

    // ProxyFactory "should set itself as factory when creating a RootProxy"
    // same factory used to build root and objects ==> OK
    ObjectProxy *object_proxy = factory.find_by_name("monitor");
    assert_true(object_proxy);

    // ProxyFactory "should build subclasses of ObjectProxy"
    assert_true(object_proxy->kind_of(ObjectProxy));

    res = object_proxy->list();
    assert_equal("[]", res.to_json());

    object_proxy->sync_children();
    millisleep(100);
    res = object_proxy->list();
    assert_equal("[\"mode\", \"tint\"]", res.to_json());
  }

  void test_local_cache_should_reflect_remote( void ) {
    Root local;
    Root remote;
    Logger logger;
    PFTLogger change_logger("log", &logger);
    MyProxyFactory factory;
    build_foobar_local_and_remote(local, remote, factory, change_logger);

    ObjectHandle bar;
    assert_true(proxy_->get_object_at("/bar", &bar));
    Value res = bar->trigger(gNilValue);
    assert_equal("45", res.to_json());

    // should have same type as remote
    assert_equal("{\"name\":\"range\", \"signature\":\"f\", \"min\":1, \"max\":127}", bar->type().to_json());
    assert_equal("{\"name\":\"range\", \"signature\":\"f\", \"min\":1, \"max\":127}", bar_->type().to_json());
  }

  void test_call_local_proxy_should_find_cached_value( void ) {
    Root local;
    Root remote;
    Logger logger;
    PFTLogger change_logger("log", &logger);
    MyProxyFactory factory;
    build_foobar_local_and_remote(local, remote, factory, change_logger);

    assert_equal("", logger.str());
    Value res = proxy_->call("/bar");
    assert_equal("45", res.to_json());
  }

  void test_change_local_proxy_should_update_remote( void ) {
    Root local;
    Root remote;
    Logger logger;
    PFTLogger change_logger("log", &logger);
    MyProxyFactory factory;
    build_foobar_local_and_remote(local, remote, factory, change_logger);

    Value res = proxy_->call("/bar", Value(33.0));
    assert_equal("45", res.to_json()); // receive cached value (async)
    millisleep(10);
    assert_equal("33", remote.call("/bar").to_json());  // remote is updated
    assert_equal("[log#bar 33]", logger.str());         // local receives 'value_changed'

    assert_equal("33", proxy_->call("/bar").to_json()); // new value is in cache now
  }

  void test_root_proxy_should_try_to_use_build_child_to_build_proxy( void ) {
    Root local;
    Root remote;
    Logger logger;
    PFTLogger change_logger("log", &logger);
    MyProxyFactory factory;
    build_foobar_local_and_remote(local, remote, factory, change_logger);

    ObjectHandle dummy_view;
    assert_true(proxy_->get_object_at("/dummy_view", &dummy_view));
    assert_equal("DummyView", dummy_view->class_name());

    ObjectHandle update;
    assert_true(proxy_->get_object_at("/dummy_view/update", &update));
    assert_equal("UpdateDummyView", update->class_name());
  }

  void test_local_cache_should_create_on_remote_create( void ) {
    Root local;
    Root remote;
    Logger logger;
    PFTLogger change_logger("log", &logger);
    MyProxyFactory factory;
    build_foobar_local_and_remote(local, remote, factory, change_logger);

    ObjectHandle bar;
    assert_false(proxy_->get_object_at("/synth", &bar));

    remote.adopt(new DummyObject("synth", gNilValue, Attribute::no_io("Super synth.")));
    millisleep(20);

    assert_true(proxy_->get_object_at("/synth", &bar));
    assert_true( bar.ptr() != NULL);

    assert_equal("Super synth.", bar->info().c_str());
  }

  // FIXME:
  // void test_local_cache_should_delete_on_remote_delete( void ) {
  //   Root local;
  //   Root remote;
  //   Logger logger;
  //   MyProxyFactory factory;
  //   build_foobar_local_and_remote(local, remote, factory, logger);
  //
  //   Object *bar = proxy_->object_at("/synth");
  //   assert_equal((Object*)NULL, bar);
  //
  //   remote.adopt(new DummyObject("synth", gNilValue, Attribute::no_io("Super synth.")));
  //   millisleep(20);
  //
  //   bar = proxy_->object_at("/synth");
  //   assert_true( bar != NULL);
  //
  //   assert_equal("Super synth.", bar->info().c_str());
  // }

private:

  /** Builds a setup where the remote tree contains real objects:
   *
   * +- foo
   * +- bar
   * +- dummy_view
   *    +- update
   *
   * the local tree has an OscCommand and builds the proxies.
   */
  void build_foobar_local_and_remote(Root &local, Root &remote, MyProxyFactory &factory,  PFTLogger &logger) {

    remote.adopt_command(new OscCommand(REMOTE_PORT));
    foo_ = remote.adopt(new DummyObject("foo", "rgb", Attribute::select_io("rgb, yuv", "This is a menu.")));
    bar_ = remote.adopt(new DummyObject("bar", 45.0, Attribute::range_io(1, 127, "This is a slider from 1 to 127.")));
    dummy_view_ = remote.adopt(new DummyObject("dummy_view", "view content", Attribute::string_io("Some dummy view.")));
    dummy_view_->adopt(new DummyObject("update", HashValue(), Attribute::hash_io("hash to update content")));

    OscCommand *cmd = local.adopt_command(new OscCommand(LOCAL_PORT));


    // build proxy
    proxy_ = (MyRootProxy*) factory.build_and_init_root_proxy(Location("oscit", Location::LOOPBACK, REMOTE_PORT));

    // sync first level (root children)
    cmd->adopt_proxy(proxy_); // sync ----> cmd ----> remote -----> cmd -----> "/.reply" ----> proxy
    millisleep(20);

    ObjectProxyLogger *object_proxy = factory.find_by_name("foo");
    object_proxy->on_value_change().connect(&logger, &PFTLogger::foo);

    object_proxy = factory.find_by_name("bar");
    object_proxy->on_value_change().connect(&logger, &PFTLogger::bar);

    object_proxy = factory.find_by_name("dummy_view");
    object_proxy->on_value_change().connect(&logger, &PFTLogger::dummy_view);
  }

  DummyObject *foo_;
  DummyObject *bar_;
  DummyObject *dummy_view_;
  MyRootProxy *proxy_;
};
