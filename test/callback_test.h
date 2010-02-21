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
#include "mock/observer_logger.h"

static Logger *s_callback_test_logger;
static void s_delete_observer_callback(Observer *observer) {
  *s_callback_test_logger << "[s_delete_observer_callback]";
  delete observer;
}

class CallbackTest : public TestHelper
{
public:
  void test_create( void ) {
    Logger logger;
    ObserverLogger a("a", &logger);
    ObserverLogger b("b", &logger);
    const char *data = "foo";
    b.adopt_callback_for_event_xyz(new TCallbackLogger<ObserverLogger, &ObserverLogger::my_callback>(&a, (void*)data, "on xyz", &logger));
    b.event_xyz();
    assert_equal("[on xyz: trigger][a: my_callback foo]", logger.str());
  }

  void test_observer_dies_before_trigger( void ) {
    Logger logger;
    ObserverLogger *a = new ObserverLogger("a", &logger);
    ObserverLogger b("b", &logger);
    const char *data = "foo";
    b.adopt_callback_for_event_xyz(new TCallbackLogger<ObserverLogger, &ObserverLogger::my_callback>(a, (void*)data, "on_xyz_callback", &logger));
    delete a;
    b.event_xyz();
    assert_equal("[a: dying][b: lock][on_xyz_callback: destroy][b: unlock]", logger.str());
  }

  void test_observable_dies_before_observer( void ) {
    Logger logger;
    ObserverLogger *a = new ObserverLogger("a", &logger);
    ObserverLogger *b = new ObserverLogger("b", &logger);
    const char *data = "foo";
    b->adopt_callback_for_event_xyz(new TCallbackLogger<ObserverLogger, &ObserverLogger::my_callback>(a, (void*)data, "on_xyz_callback", &logger));
    delete b;
    assert_equal("[b: dying][on_xyz_callback: destroy][a: lock][a: unlock]", logger.str());
    logger.str("");
    delete a;
    assert_equal("[a: dying]", logger.str());
  }

  void test_delete_observer_callback_master_dies_first( void ) {
    Logger logger;
    ObserverLogger *master = new ObserverLogger("master", &logger);
    ObserverLogger *slave  = new ObserverLogger("slave",  &logger);

    master->adopt_callback_on_destroy(
      new CallbackToDelete(slave)
    );

    delete master;
    assert_equal("[master: dying][slave: lock][slave: unlock][slave: dying]", logger.str());
    logger.str("");
  }

  void test_delete_observer_callback_slave_dies_first( void ) {
    Logger logger;
    ObserverLogger *master = new ObserverLogger("master", &logger);
    ObserverLogger *slave  = new ObserverLogger("slave",  &logger);

    master->adopt_callback_on_destroy(
      new CallbackToDelete(slave)
    );

    delete slave;
    assert_equal("[slave: dying][master: lock][master: unlock]", logger.str());
    logger.str("");

    delete master;
    assert_equal("[master: dying]", logger.str());
  }

  void test_delete_observer_callback_master_dies_first_use_static( void ) {
    Logger logger;
    ObserverLogger *master = new ObserverLogger("master", &logger);
    ObserverLogger *slave  = new ObserverLogger("slave",  &logger);

    s_callback_test_logger = &logger;
    master->adopt_callback_on_destroy(
      new CallbackToDelete(slave, NULL, &s_delete_observer_callback)
    );

    delete master;
    assert_equal("[master: dying][slave: lock][slave: unlock][s_delete_observer_callback][slave: dying]", logger.str());
    logger.str("");
  }

  void test_delete_observer_callback_change_master( void ) {
    Logger logger;
    ObserverLogger *master1 = new ObserverLogger("master1", &logger);
    ObserverLogger *master2 = new ObserverLogger("master2", &logger);
    ObserverLogger *slave   = new ObserverLogger("slave",  &logger);

    master1->adopt_callback_on_destroy(
      new CallbackToDelete(slave, master1)
    );

    slave->delete_produced_callbacks_with_data(master1);
    assert_equal("[master1: lock][master1: unlock]", logger.str());
    logger.str("");

    master2->adopt_callback_on_destroy(
      new CallbackToDelete(slave, master2)
    );

    delete master1;
    assert_equal("[master1: dying]", logger.str());
    logger.str("");

    delete master2;
    assert_equal("[master2: dying][slave: lock][slave: unlock][slave: dying]", logger.str());
    logger.str("");
  }
};

