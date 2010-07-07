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
#include "mock/dummy_object.h"
#include "mock/observer_logger.h"

class ObjectTest : public TestHelper
{
public:
  void test_create( void ) {
    DummyObject a("a", 1);
    assert_equal("a", a.name());
  }

  void test_rename( void ) {
    Object base;
    Object * foo = base.adopt(new Object("foo"));
    Object * bar = foo->adopt(new Object("bar"));

    assert_equal("/foo"    , foo->url() );
    assert_equal("/foo/bar", bar->url() );

    foo->set_name("super");

    assert_equal("/super"    , foo->url() );
    assert_equal("/super/bar", bar->url() );
  }

  void test_get_child( void ) {
    Object base;
    Object * one = base.adopt(new Object("one")); // This is the prefered way of creating sub-objects.
    Object * sub = one->adopt(new Object("sub"));
    ObjectHandle handle;

    assert_equal("",         base.url());
    assert_equal("/one",     one->url());
    assert_equal("/one/sub", sub->url());

    assert_true(base.get_child("one", &handle));
    assert_equal(one, handle.ptr());

    handle = NULL;
    assert_false(base.get_child("foo", &handle));
    assert_equal((Object*)NULL, handle.ptr());

    one->set_name("foo");

    handle = NULL;
    assert_false(base.get_child("one", &handle));
    assert_equal((Object*)NULL, handle.ptr());

    assert_true(base.get_child("foo", &handle));
    assert_equal(one, handle.ptr());
  }

  void test_first_child( void ) {
    Object base("base");
    Object * one = base.adopt(new Object("one"));
    Object * two = base.adopt(new Object("aaa"));
    ObjectHandle obj;

    assert_true(base.first_child(&obj));
    assert_equal(one, obj.ptr());

    obj = NULL; // release handle

    delete one;

    assert_true(base.first_child(&obj));
    assert_equal(two, obj.ptr());
  }

  void test_set_parent_same_name_as_sibling( void ) {
    Object base;
    Object * child1 = new Object("foo");
    Object * child2 = new Object("bar");
    Object * child3 = new Object("foo");

    assert_equal(""   , base.url()    );
    assert_equal("foo", child1->url() );
    assert_equal("bar", child2->url() );
    assert_equal("foo", child3->url() );

    child1->set_parent(&base);

    assert_equal(""    , base.url()    );
    assert_equal("/foo", child1->url() );
    assert_equal("bar" , child2->url() );
    assert_equal("foo" , child3->url() );

    child2->set_parent(child1);

    assert_equal(""        , base.url()    );
    assert_equal("/foo"    , child1->url() );
    assert_equal("/foo/bar", child2->url() );
    assert_equal("foo"     , child3->url() );

    child3->set_parent(&base);

    assert_equal(""        , base.url()    );
    assert_equal("/foo"    , child1->url() );
    assert_equal("/foo/bar", child2->url() );
    assert_equal("/foo-1"  , child3->url() );
  }

  void test_build_child( void ) {
    Value error;
    Object base;
    Object * carrot = base.adopt(new DummyObject("dummy", 0.0));
    ObjectHandle handle;
    assert_false(carrot->build_child(std::string("something"), gNilValue, &error, &handle));
    assert_equal((Object*)NULL, handle.ptr());

    assert_true(carrot->build_child(std::string("special"), gNilValue, &error, &handle));
    assert_true( handle.ptr() != NULL );
    assert_equal("/dummy/special", handle->url());
  }

  void test_set_should_call_sub_objects( void ) {
    Object base;
    DummyObject *one = base.adopt(new DummyObject("one", 123.0));
    DummyObject *two = base.adopt(new DummyObject("two", 123.0));
    Value res = base.set(JsonValue("one:10.0 two:22.22"));
    assert_true(res.is_hash());
    assert_equal("{\"one\":10, \"two\":22.22}", res.to_json());
    assert_equal(10.0, one->real());
    assert_equal(22.22, two->real());
  }

  void test_set_should_merge_at_keys_in_attributes( void ) {
    Object base("", Attribute::range_io("Some info", -1, 1));
    base.adopt(new DummyObject("one", 123.0));
    base.adopt(new DummyObject("two", 123.0));
    Value res = base.set(JsonValue("one:10.0 two:22.22 @type:{min:3, max:10}"));
    assert_true(res.is_hash());
    assert_equal("{\"one\":10, \"two\":22.22, \"@type\":{\"min\":3, \"max\":10}}", res.to_json());
    assert_equal("3",  base.attributes()[Attribute::TYPE][Attribute::MIN].to_json());
    assert_equal("10", base.attributes()[Attribute::TYPE][Attribute::MAX].to_json());
    // make sure we execute a deep merge, not a replace on the type hash
    assert_equal("\"f\"", base.attributes()[Attribute::TYPE][Attribute::SIGNATURE].to_json());
  }

  void test_call_set_method_return_bool( void ) {
    Object base;
    DummyObject * one = base.adopt(new DummyObject("one", 123.0));
    DummyObject * two = base.adopt(new DummyObject("two", 123.0));
    assert_true(base.set_all_ok(JsonValue("one:10.0 two:22.22")));
    Value res = base.set(JsonValue("one:1.0 four:4.0"));
    assert_false(base.set_all_ok(JsonValue("one:1.0 four:4.0")));
    assert_equal(1.0,   one->real());
    assert_equal(22.22, two->real());
  }

  void test_hash_type_id( void ) {
    Object hash("foo", Attribute::hash_io("bar"));
    Object matr("foo", Attribute::matrix_io("bar"));
    assert_false(hash.type_id() == matr.type_id());
    assert_false(hash.type_id() == matr.type_id());
    assert_equal("H", hash.type()[Attribute::SIGNATURE].str());
    assert_equal("M", matr.type()[Attribute::SIGNATURE].str());
  }

  void test_list_with_attributes( void ) {
    Object base;
    base.adopt(new DummyObject("mode", "rgb", Attribute::select_io("This is a menu.", "rgb, yuv")));
    base.adopt(new DummyObject("tint", 45.0, Attribute::range_io("This is a slider from 1 to 127.", 1, 127)));
    Value res = base.list_with_attributes();
    assert_equal(res.type_tag(), "sHsH");
    assert_equal("mode", res[0].str());
    assert_equal("tint", res[2].str());
  }

  void should_get_child_from_position( void ) {
    Object base("base");
    Object *one = base.adopt(new Object("one"));
    assert_equal(1, base.children_count());
    Object *two = base.adopt(new Object("aaa"));
    assert_equal(2, base.children_count());
    ObjectHandle obj;

    assert_true(base.get_child(0, &obj));
    assert_equal(one, obj.ptr());

    assert_true(base.get_child(1, &obj));
    assert_equal(two, obj.ptr());

    one->release(); // delete
    assert_equal(1, base.children_count());

    assert_true(base.get_child(0, &obj));
    assert_equal(two, obj.ptr());

    assert_false(base.get_child(1, &obj));
    obj = NULL; // release handle;

    two->release(); // delete
    assert_equal(0, base.children_count());
    assert_false(base.get_child(0, &obj));
  }

  void test_should_keep_last( void ) {
    Object base("base");
    Object *last = base.adopt(new Object("last"));
    last->set_keep_last(true);
    assert_equal(1, base.children_count());

    base.adopt(new Object("one"));
    assert_equal(2, base.children_count());
    assert_equal("[\"one\", \"last\"]", base.list().to_json());

    base.adopt(new Object("two"));
    assert_equal(3, base.children_count());
    assert_equal("[\"one\", \"two\", \"last\"]", base.list().to_json());

    base.adopt(new Object("last2", Attribute::default_io(), true));
    assert_equal(4, base.children_count());
    assert_equal("[\"one\", \"two\", \"last\", \"last2\"]", base.list().to_json());
  }

  void test_to_hash_should_list_children( void ) {
    Object base("base");
    base.adopt(new DummyObject("legs", 100));
    base.adopt(new DummyObject("arms", 60));
    assert_equal("{\"@info\":\"Container.\", \"legs\":100, \"arms\":60}", base.to_hash().to_json());
  }

  void test_nested_to_hash_should_list_children( void ) {
    Object base("base");
    base.adopt(new DummyObject("legs", 100));
    base.adopt(new DummyObject("arms", 60));
    Object *patch = base.adopt(new Object("group"));
    patch->adopt(new DummyObject("Africa", Value("Unite!"), Attribute::string_io("Something")));
    assert_equal("{\"@info\":\"Container.\", \"legs\":100, \"arms\":60, \"group\":{\"@info\":\"Container.\", \"Africa\":\"Unite!\"}}", base.to_hash().to_json());
  }

  void test_on_delete_notification( void ) {
    Object *base = new Object("base");
    DummyObject *width  = base->adopt(new DummyObject("width", 100));

    Logger logger;
    ObserverLogger observer("observer", &logger);
    width->on_delete().connect(&observer, &ObserverLogger::event);

    assert_equal("", logger.str());
    delete base;

    assert_equal("[observer: \"base/width\"]", logger.str());
  }

  void test_on_delete_delete( void ) {
    Object *base = new Object("base");
    DummyObject *width  = base->adopt(new DummyObject("width", 100));

    Logger logger;
    ObserverLogger *observer = new ObserverLogger("observer", &logger);
    width->on_delete().connect(observer, &ObserverLogger::delete_this);

    assert_equal("", logger.str());
    delete base;

    assert_equal("[observer: deleting][observer: deleted]", logger.str());
  }

  // set_type is not a good idea. It should be immutable (or maybe I'm wrong, so I leave the test here)
  //void test_set_type( void ) {
  //  DummyObject one("one", 123.0);
  //  assert_equal("fffs", one.type().type_tag()); // RangeIO
  //  assert_equal(0.0, one.type()[0].r); // current
  //  assert_equal(0.0, one.type()[1].r);   // min
  //  assert_equal(127.0, one.type()[2].r); // max
  //  assert_equal("lux", one.type()[3].str()); // unit
  //  assert_equal(DUMMY_OBJECT_INFO, one.type()[4].str()); // info
  //
  //  Value type(TypeTag("fss"));
  //  assert_false(one.set_type(type));
  //  assert_equal("fffs", one.type().type_tag());
  //  assert_equal("lux", one.type()[3].str()); // unit
  //  type.set(0.0);
  //  type.push_back(0).push_back(5).push_back("eggs").push_back("I'm a hen !");
  //  assert_true(one.set_type(type));
  //  assert_equal("fffs", one.type().type_tag());
  //  assert_equal("eggs", one.type()[1].str());
  //}
};