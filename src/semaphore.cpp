#include "oscit/semaphore.h"

namespace oscit {

Semaphore::Semaphore(int resource_count) : resource_count_(resource_count), semaphore_(NULL) {
//#ifdef __macosx__
  semaphore_ = sem_open("oscit::Semaphore", O_CREAT, 0, resource_count_);
  if (semaphore_ == NULL) {
    fprintf(stderr, "Could not open semaphore 'oscit::Semaphore' (%s)\n", strerror(errno));
  } else {
    // transform this process wide semaphore into an unamed semaphore.
    if (sem_unlink("oscit::Semaphore") < 0) {
      fprintf(stderr, "Could not unlink semaphore 'oscit::Semaphore' (%s)\n", strerror(errno));
    }
  }
//#else
//  if (sem_init(&semaphore_, 0, 0) < 0) {
//    fprintf(stderr, "Could not init semaphore (%s)\n", strerror(errno));
//  }
//#endif
}

Semaphore::~Semaphore() {
  if (semaphore_) {
//#ifdef __macosx__
    sem_close(semaphore_);
//#else
//    sem_destroy(*semaphore_);
//#endif
  }
}


}  // oscit