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

#ifndef _TEST_HELPER_H_
#define _TEST_HELPER_H_
#include <cxxtest/TestSuite.h>
#include "oscit/oscit.h"
#include "oscit/thread.h"
#include "oscit/file.h"
#include <ostream>

using namespace oscit;

#define TEST_FIXTURES_PATH "../test/fixtures"

#define assert_equal(x,y) _assert_equal(__FILE__,__LINE__,#y,x,y)
#define assert_true(e) _TS_ASSERT(__FILE__,__LINE__,e)
#define assert_false(e) _TS_ASSERT(__FILE__,__LINE__,!(e))

#define _OSCIT_ASSERT_EQUALS(f,l,cmd,y,x) { _TS_TRY { CxxTest::doAssertEquals( (f), (l), (cmd), (x), #y, (y), (0) ); } __TS_CATCH(f,l) }

class TestHelper : public CxxTest::TestSuite
{
public:
  TestHelper() : saved_content_(10) {}

protected:

  void _assert_equal(const char * file, int lineno, const char * descr, Real expected, Real found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  void _assert_equal(const char * file, int lineno, const char * descr, int expected, int found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  void _assert_equal(const char * file, int lineno, const char * descr, uint expected, uint found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  template<class T>
  void _assert_equal(const char * file, int lineno, const char * descr, int expected, T found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), (T)expected, found);
  }

  template<class T>
  void _assert_equal(const char * file, int lineno, const char * descr, T expected, T found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  void _assert_equal(const char * file, int lineno, const char * descr, unsigned long expected, unsigned long found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  void _assert_equal(const char * file, int lineno, const char * descr, const char * expected, const char * found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), std::string(expected), std::string(found));
  }

  void _assert_equal(const char * file, int lineno, const char * descr, const char * expected, const std::string &found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), std::string(expected), found);
  }

  void _assert_equal(const char * file, int lineno, const char * descr, const std::string &expected, const std::string &found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  void _assert_equal(const char * file, int lineno, const char * descr, void * expected, void * found)
  {
    _OSCIT_ASSERT_EQUALS( file, lineno, TS_AS_STRING(descr), expected, found);
  }

  //// time based utilities
  static void millisleep(float microseconds) {
    Thread::millisleep(microseconds);
  }


  //// file related helpers
  std::string fixture_path(const char *path) {
    return std::string(TEST_FIXTURES_PATH).append("/").append(path);
  }

  void preserve(const char *path) {
    preserve(std::string(path));
  }

  void preserve(const std::string &path) {
    if (!saved_content_.has_key(path)) {
      File file(path);
      Value content = file.read();
      if (content.is_string())
        saved_content_.set(path, content.str());
    }
  }

  void restore(const char *path) {
    restore(std::string(path));
  }

  void restore(const std::string &path) {
    std::string content;
    if (saved_content_.get(path, &content)) {
      File file(path);
      file.write(content);
    }
  }

private:
  THash<std::string, std::string> saved_content_;
};

#endif // _TEST_HELPER_H_
