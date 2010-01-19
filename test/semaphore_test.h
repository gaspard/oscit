#include "test_helper.h"
#include "oscit/mutex.h"
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
    runner.start_thread<SemaphoreTest, &SemaphoreTest::multiple_semaphore_thread>(NULL);  // waits for 'thread_ready'
    logger_ << "[0:start done]";
    s1_.release();
    runner.join();
    logger_ << "[0:join]";
    assert_equal("[1:started][0:start done][1:continue][0:join]", logger_.str());
  }

  void multiple_semaphores_thread(Thread *runner) {
    millisleep(100);
    logger_ << "[1:started]";
    runner->thread_ready();
    s1_->acquire();
    logger_ << "[1:continue]";
  }
private:
  std::ostringstream logger_;
  Semaphore s1_;
};
