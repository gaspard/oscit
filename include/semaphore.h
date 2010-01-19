#ifndef OSCIT_INCLUDE_OSCIT_SEMAPHORE_H_
#define OSCIT_INCLUDE_OSCIT_SEMAPHORE_H_
#include <assert.h>
#include <semaphore.h>
#include <errno.h>   // errno
#include <string.h>  // strerror
#include "oscit/non_copyable.h"
#include "oscit/mutex.h"

namespace oscit {

class Semaphore : private NonCopyable {
public:
  Semaphore(int resource_count = 0);
  
  ~Semaphore();
  
  /** Get resource or wait until it is available.
   */
  inline void acquire() {
    if (sem_wait(semaphore_) < 0) {
      fprintf(stderr, "Could not increase semaphore (%s)\n", strerror(errno));
    }
  }
  
  /** Release resource.
   */
  inline void release() {
    if (sem_post(semaphore_) < 0) {
      fprintf(stderr, "Could not decrease semaphore (%s)\n", strerror(errno));
    }
  }
  
  /** Acquire multiple resources at the same time.
   */
  inline void acquire(size_t count) {
    assert(count <= resource_count_);
    ScopedLock lock(acquire_mutex_);
    for (size_t i=0; i < count; ++i) {
      this->acquire();
    }
  }
  
  /** Release multiple resources at the same time.
   */
  inline void release(size_t count) {
    assert(count <= resource_count_);
    for (size_t i=0; i < count; ++i) {
      this->release();
    }
  }
  
  /** Acquire all resources protected by this semaphore
   */
  inline void acquire_all() {
    acquire(resource_count_);
  }
  
  /** Release all resources previously acquired with a call to 'acquire_all'.
   */
  inline void release_all() {
    release(resource_count_);
  }
  
 private:
  size_t resource_count_;
  Mutex acquire_mutex_;
  sem_t *semaphore_;
};

}  // oscit
#endif  // OSCIT_INCLUDE_OSCIT_SEMAPHORE_H_