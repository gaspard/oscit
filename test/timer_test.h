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
#include "oscit/timer.h"
#include "mock/logger.h"

class TimerTest : public TestHelper
{
public:
  void setUp() {
    sleep_in_loop_ = 0;
    counter_ = 0;
    //std::cout << "\n\n";
  }

  void should_fire_between_start_and_stop( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);

    timer.start(10);
    millisleep(35);
    timer.stop();
    assert_equal(4, counter_);
    millisleep(20);
    assert_equal(4, counter_);
  }
// FIXME: how to test without printing results of timer ?
// Run 50 times and ensure now - start = 50 * elapsed ?
  void should_adapt_sleep_time_to_loop_duration( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);
    sleep_in_loop_ = 6;
    timer.start(10);
    millisleep(35);
    timer.stop();
    assert_equal(4, counter_);
  }

  void should_change_interval_but_stay_in_sync_on_set_interval( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);
    timer.start(20);
    millisleep(30); // 0 .. 20
    timer.set_interval(8);
    millisleep(12); // 0 .. 20 (28) [30:change] .. 36
    timer.set_interval(10);
    millisleep(30); // 0 .. 20 (28) [30:change] .. 36 .. [42:change] 46 .. 56 .. 66 .. [72:end]
    timer.stop();
    assert_equal(6, counter_);
  }

  void should_not_stay_in_sync_on_start( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);
    timer.start(20);
    millisleep(30); // 0 .. 20
    timer.start(8);
    millisleep(12); // 0 .. 20 .. [30:change] .. 38
    timer.start(10);
    millisleep(35); // 0 .. 20 .. [30:change] .. 38 .. [42:change] .. 52 .. 62 .. 72
    timer.stop();
    assert_equal(8, counter_);
  }

  void should_stop_on_delete( void ) {
    Timer<TimerTest, &TimerTest::loop> *timer = new Timer<TimerTest, &TimerTest::loop>(this);
    timer->start(10);
    millisleep(35);
    assert_equal(4, counter_);
    delete timer;
    millisleep(35);
    assert_equal(4, counter_);
  }

  void should_ignore_restart( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);
    timer.start(10);
    timer.start(5);
  }

  void should_not_wait_when_reducing_interval( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);
    timer.start(5000);     // 0
    millisleep(12);
    timer.set_interval(10);
    millisleep(15);        // 0 .. (10) [12:change = should not fire] .. 20 .. [25:stop]
    timer.stop();
    assert_equal(2, counter_);
  }

  void test_create_stop_start_again( void ) {
    Timer<TimerTest, &TimerTest::loop> timer(this);
    timer.start(10);
    millisleep(15);
    timer.stop();
    millisleep(10);
    assert_equal(2, counter_);
    timer.start(10);
    millisleep(15);
    timer.stop();
    assert_equal(4, counter_);
  }

  void loop() {
    //std::cout << time_ref_.elapsed() << "\n";
    counter_ += 1;
  }

private:
  time_t sleep_in_loop_;
  TimeRef time_ref_;
  int counter_;
};

