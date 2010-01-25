#include "test_helper.h"
#include "oscit/semaphore.h"
#include <sstream>

class SemaphoreTest : public TestHelper
{
public:
  SemaphoreTest() : logger_(std::ostringstream::out) {}

  void test_multiple_semaphores( void ) {
    logger_.str("");
    Semaphore s2, s3;
    Thread runner;
    s2.release();  // increment semaphore resource ==> 1
    s3.release();  // increment semaphore resource ==> 1


    // s1_ should not be 2. It would if all semaphores were global (same name 'oscit::Semaphore').
    runner.start_thread<SemaphoreTest, &SemaphoreTest::multiple_semaphore_thread>(this, NULL);
                                // wait for [1]
    logger_ << "[0:start done]";
    s1_.release();              // [A]
    runner.join();              // wait for [2]
    logger_ << "[0:join]";
    assert_equal("[1:started][0:start done][1:continue][0:join]", logger_.str());
  }

  void should_increase_semaphore_on_each_release( void ) {
    Semaphore s;
    s.release();
    s.release(); // ==> count is 2
    s.acquire();
    s.acquire(); // ==> count is now 0
    // should not lock
  }

  void multiple_semaphore_thread(Thread *runner) {
    millisleep(100); // sleep does not give main thread execution
    logger_ << "[1:started]";
    runner->thread_ready();      // [1]
    s1_.acquire();               // wait for [A]
    logger_ << "[1:continue]";
                                 // [2]
  }
private:
  std::ostringstream logger_;
  Semaphore s1_;
};
