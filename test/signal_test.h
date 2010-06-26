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

#include <cxxtest/TestSuite.h>
#include "test_helper.h"

#include "oscit/signal.h"

// BT is for BoostTest in case you wonder...
class SenderBT {
public:
  void message(const char *message) {
    some_signal_.send(Value(message));
  }

  Signal some_signal_;
};

//
///** The goal of this test is to make sure boost compiles and behaves as expected (and we
// * understand how to use it).
// */
class SignalTest : public TestHelper {
public:

  void test_true( void ) {
    assert_true(true);
  }

  void test_receiver_dies_first( void ) {
    SenderBT root;
    Logger oss;
    ObserverLogger *observer = new ObserverLogger("a", &oss);

    // nothing send (slot empty)
    root.message("message A");

    root.some_signal_.connect<ObserverLogger, &ObserverLogger::event>(observer);

    // message sent
    root.message("message B");

    delete observer;

    // no message sent
    root.message("message C");

    assert_equal("[a: \"message B\"][a: deleted]", oss.str());
  }

  void test_sender_dies_first( void ) {
    SenderBT *root = new SenderBT;
    Logger oss;
    ObserverLogger *observer = new ObserverLogger("a", &oss);

    // nothing send (slot empty)
    root->message("message A");

    root->some_signal_.connect<ObserverLogger, &ObserverLogger::event>(observer);

    // message sent
    root->message("message B");

    delete root;
    delete observer;

    assert_equal("[a: \"message B\"][a: deleted]", oss.str());
  }
};

