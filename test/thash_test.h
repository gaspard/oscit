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
#include "oscit/thash.h"

class DummyTHashTest : private NonCopyable {
public:
  DummyTHashTest() {}
};

class THashTest : public TestHelper
{
public:
  void test_set_get( void ) {
    THash<std::string, std::string> hash(10);
    std::string res;

    hash.set(std::string("hello"), std::string("world"));
    hash.set(std::string("my"),    std::string("mum"));

    assert_true(hash.get("hello", &res));
    assert_equal("world", res);
    assert_true(hash.get("my", &res));
    assert_equal("mum", res);
    assert_false(hash.get("money", &res));
  }

  void test_set_get_ptr( void ) {
    THash<std::string, DummyTHashTest*> hash(10);
    DummyTHashTest *a = new DummyTHashTest;
    DummyTHashTest *b = new DummyTHashTest;
    DummyTHashTest *res;

    hash.set(std::string("hello"), a);
    hash.set(std::string("my"),    b);

    assert_true(hash.get("hello", &res));
    assert_equal(a, res);
    assert_true(hash.get("my", &res));
    assert_equal(b, res);
    assert_false(hash.get("money", &res));
  }

  void test_set_get_int_key( void ) {
    THash<int, std::string> hash(10);
    std::string res;

    hash.set(23, std::string("hello"));
    hash.set(-2, std::string("my"));

    assert_true(hash.get(23, &res));
    assert_equal("hello", res);
    assert_true(hash.get(-2, &res));
    assert_equal("my", res);
    assert_false(hash.get(34, &res));
  }

  void test_set_get_empty( void ) {
    THash<std::string, std::string> hash(10);
    std::string res;

    hash.set(std::string(""), std::string("empty"));
    hash.set(std::string("other"),    std::string("string"));

    assert_true(hash.get("", &res));
    assert_equal("empty", res);
    assert_true(hash.get("other", &res));
    assert_equal("string", res);
  }

  void test_get_key( void ) {
    THash<std::string, std::string> hash(10);
    std::string res;

    hash.set(std::string("hello"), std::string("world"));
    hash.set(std::string("my"),    std::string("mum"));

    assert_true(hash.get_key("world", &res));
    assert_equal("hello", res);
    assert_true(hash.get_key("mum", &res));
    assert_equal("my", res);
    assert_false(hash.get_key("money", &res));
  }

  void test_has_key( void ) {
    THash<std::string, std::string> hash(10);
    std::string res;

    hash.set(std::string("hello"), std::string("world"));
    hash.set(std::string("my"),    std::string("mum"));

    assert_true(hash.has_key("hello"));
    assert_true(hash.has_key("my"));
    assert_false(hash.has_key("bad key"));
    assert_false(hash.has_key("world"));
  }

  void test_keys( void ) {
    THash<std::string, std::string> hash(10);
    const std::list<std::string> * keys = &hash.keys();

    hash.set(std::string("hello"), std::string("world"));
    assert_equal(1, keys->size());
    assert_equal("hello", keys->front());

    hash.set(std::string("my"),    std::string("mum"));
    assert_equal(2, keys->size());
    assert_equal("hello", keys->front());
    assert_equal("my", keys->back());

    hash.remove(std::string("bob")); // does nothing
    assert_equal("hello", keys->front());
    assert_equal("my", keys->back());

    hash.remove(std::string("hello"));
    assert_equal(1, keys->size());
    assert_equal("my", keys->front());
  }

  void test_clear( void ) {
    THash<std::string, std::string> hash(10);
    std::string res;

    hash.set(std::string("a"), std::string("world"));
    hash.set(std::string("b"), std::string("mum"));



    assert_equal(2, hash.keys().size());

    assert_true(hash.get("a", &res));
    assert_equal( res,  std::string("world") );
    assert_true(hash.get("b", &res));
    assert_equal( res,  std::string("mum"  ) );

    hash.clear();
    assert_equal(0, hash.keys().size());
    assert_false(hash.get("a", &res));
    assert_false(hash.get("b", &res));
  }

  void test_clear_int( void ) {
    THash<std::string, int> hash(10);
    int res;

    hash.set(std::string("a"), 1);
    hash.set(std::string("b"), 2);
    hash.set(std::string("c"), 3);


    assert_equal(3, hash.size());

    assert_true(hash.get("a", &res));
    assert_equal(1, res);
    assert_true(hash.get("b", &res));
    assert_equal(2, res);
    assert_true(hash.get("c", &res));
    assert_equal(3, res);

    hash.clear();
    assert_equal(0,  hash.size());
    assert_false(hash.get("a", &res));
    assert_false(hash.get("b", &res));
    assert_false(hash.get("c", &res));
  }

  void test_hashId_( void ) {
    assert_false(hashId("a") == hashId("b"));
    assert_false(hashId("one") == hashId("two"));
    assert_false(hashId("longname_a") == hashId("longname_b"));
    assert_false(hashId("longname_a") == hashId("longname_c"));
    assert_false(hashId("longnamelong_a") == hashId("longnamelong_c"));
  }

  void test_N_type_id( void ) {
    assert_false(hashId("N") == hashId("T"));
    assert_false(hashId("N") == hashId("F"));
    assert_false(hashId("N") == hashId("f"));
    assert_false(hashId("N") == hashId("s"));
    assert_false(hashId("N") == hashId("H"));
    assert_false(hashId("N") == hashId("M"));
    assert_false(hashId("N") == hashId("m"));
    assert_false(hashId("N") == hashId("*"));
  }

  void test_T_type_id( void ) {
    assert_false(hashId("T") == hashId("N"));
    assert_false(hashId("T") == hashId("F"));
    assert_false(hashId("T") == hashId("f"));
    assert_false(hashId("T") == hashId("s"));
    assert_false(hashId("T") == hashId("H"));
    assert_false(hashId("T") == hashId("M"));
    assert_false(hashId("T") == hashId("m"));
    assert_false(hashId("T") == hashId("*"));
  }

  void test_F_type_id( void ) {
    assert_false(hashId("F") == hashId("N"));
    assert_false(hashId("F") == hashId("T"));
    assert_false(hashId("F") == hashId("f"));
    assert_false(hashId("F") == hashId("s"));
    assert_false(hashId("F") == hashId("H"));
    assert_false(hashId("F") == hashId("M"));
    assert_false(hashId("F") == hashId("m"));
    assert_false(hashId("F") == hashId("*"));
  }

  void test_f_type_id( void ) {
    assert_false(hashId("f") == hashId("N"));
    assert_false(hashId("f") == hashId("T"));
    assert_false(hashId("f") == hashId("F"));
    assert_false(hashId("f") == hashId("s"));
    assert_false(hashId("f") == hashId("H"));
    assert_false(hashId("f") == hashId("M"));
    assert_false(hashId("f") == hashId("m"));
    assert_false(hashId("f") == hashId("*"));
  }

  void test_s_type_id( void ) {
    assert_false(hashId("s") == hashId("N"));
    assert_false(hashId("s") == hashId("T"));
    assert_false(hashId("s") == hashId("F"));
    assert_false(hashId("s") == hashId("f"));
    assert_false(hashId("s") == hashId("H"));
    assert_false(hashId("s") == hashId("M"));
    assert_false(hashId("s") == hashId("m"));
    assert_false(hashId("s") == hashId("*"));
  }

  void test_H_type_id( void ) {
    assert_false(hashId("H") == hashId("N"));
    assert_false(hashId("H") == hashId("f"));
    assert_false(hashId("H") == hashId("s"));
    assert_false(hashId("H") == hashId("M"));
    assert_false(hashId("H") == hashId("m"));
    assert_false(hashId("H") == hashId("*"));
  }

  void test_M_type_id( void ) {
    assert_false(hashId("M") == hashId("N"));
    assert_false(hashId("M") == hashId("T"));
    assert_false(hashId("M") == hashId("F"));
    assert_false(hashId("M") == hashId("f"));
    assert_false(hashId("M") == hashId("s"));
    assert_false(hashId("M") == hashId("H"));
    assert_false(hashId("M") == hashId("m"));
    assert_false(hashId("M") == hashId("*"));
  }

  void test_m_type_id( void ) {
    assert_false(hashId("m") == hashId("N"));
    assert_false(hashId("m") == hashId("T"));
    assert_false(hashId("m") == hashId("F"));
    assert_false(hashId("m") == hashId("f"));
    assert_false(hashId("m") == hashId("s"));
    assert_false(hashId("m") == hashId("H"));
    assert_false(hashId("m") == hashId("M"));
    assert_false(hashId("m") == hashId("*"));
  }

  void test_any_type_id( void ) {
    assert_false(hashId("*") == hashId("N"));
    assert_false(hashId("*") == hashId("T"));
    assert_false(hashId("*") == hashId("F"));
    assert_false(hashId("*") == hashId("f"));
    assert_false(hashId("*") == hashId("s"));
    assert_false(hashId("*") == hashId("H"));
    assert_false(hashId("*") == hashId("m"));
    assert_false(hashId("*") == hashId("M"));
  }

  void test_copy( void ) {
    THash<std::string, std::string> hash(10);
    THash<std::string, std::string> hash2(10);
    std::string res;

    hash.set(std::string("hello"), std::string("world"));
    hash.set(std::string("my"),    std::string("mum"));

    // copy
    hash2 = hash;

    // change hash
    hash.set(std::string("hello"), std::string("planet"));

    assert_true(hash.get("hello", &res));
    assert_equal("planet", res);
    assert_true(hash.get("my", &res));
    assert_equal("mum", res);

    assert_true(hash2.get("hello", &res));
    assert_equal("world", res);
    assert_true(hash2.get("my", &res));
    assert_equal("mum", res);
  }

  void test_set_get_pointer( void ) {
    THash<std::string, std::string> hash(10);
    const std::string *res;

    hash.set(std::string("hello"), std::string("world"));
    hash.set(std::string("my"),    std::string("mum"));

    assert_true(hash.get("hello", &res));
    assert_equal("world", *res);
    assert_true(hash.get("my", &res));
    assert_equal("mum", *res);
    assert_false(hash.get("money", &res));
  }


  void test_equal( void ) {
    THash<std::string, int> a(10);
    THash<std::string, int> b(10);
    assert_equal(a, b);
    a.set(std::string("one"), 1);
    assert_false(a == b);
    b.set(std::string("one"), 1);
    assert_equal(a, b);
    a.set(std::string("two"), 2);
    assert_false(a == b);
    b.set(std::string("two"), 2);
    assert_equal(a, b);
    a.set(std::string("two"), 4);
    assert_false(a == b);
  }
};