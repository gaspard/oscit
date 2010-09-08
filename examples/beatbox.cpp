// compile with
// g++ -D__macosx__ -I../include ../build/liboscit.a beatbox.cpp -o beatbox
// in order to find 'beatbox.json' file (view), beatbox should be launched from within oscit/examples folder

#include <stdio.h>
#include <stdlib.h> // atoi
#include <iostream>

#include "oscit/oscit.h"
using namespace oscit;


#define OSC_PORT 7021
#define VIEW_PATH "."

class ValueDisplay : public Object
{
public:
  ValueDisplay(const char *name, Real current_value, uint *sleepy)
      : Object(name, Oscit::range_io("Current value", 0, 512)),
        value_(current_value),
        sleepy_(sleepy) {}

  /** This is the method triggered in response to the object's url.
   *  In this example this url is "/tempo".
   */
  virtual const Value trigger(const Value &val) {
    if (val.is_real()) {
      value_.r = val.r;

      // when the value goes to 0: divide sleepiness by two
      // when it goes to 512: multiply by two
      // this is to test changing network latencies
      if (val.r == 0) {
        *sleepy_ = *sleepy_ / 2;
      } else if (val.r == 512) {
        *sleepy_ = *sleepy_ * 2;
      }
    }

    std::cout << name_ << ": " << value_ << std::endl;

    // oscit convention is to return current value
    return value_;
  }

private:
  RealValue value_;
  uint *sleepy_;
};

class SleepyOscCommand : public OscCommand {
public:
  SleepyOscCommand(const char *protocol, const char *service_type, uint *sleepy)
      : OscCommand(protocol, service_type),
        sleepy_(sleepy) {}

  virtual void receive(const Url &url, const Value &val) {
    // simulate a slow network
    if (*sleepy_) {
      std::cout << "sleep: " << *sleepy_ << "\n";
      Thread::millisleep(*sleepy_);
    }
    Command::receive(url, val);
  }

private:
  uint *sleepy_;
};

static bool gRun = true;
static uint gSleepy = 0;

void terminate(int sig) {
  gRun = false;
}


int main(int argc, char * argv[]) {
  Root root("beatbox");
  Value test;

  if (argc > 1) {
    gSleepy = atoi(argv[1]);
  }

  // open osc command on port OSC_PORT
  root.adopt_command(new SleepyOscCommand("oscit", OSCIT_SRV_TYPE, &gSleepy));

  // create '/tempo' url
  root.adopt(new ValueDisplay("tempo", 120, &gSleepy));

  // create '/other/deeply/nested/slider' url
  Object *tmp = root.adopt(new Object("other"));
  tmp = tmp->adopt(new Object("deeply"));
  tmp = tmp->adopt(new Object("nested"));
  tmp->adopt(new ValueDisplay("slider", 115, &gSleepy));

  // create '/views' url
  Value error;
  if (!root.expose_views(VIEW_PATH, &error)) {
    std::cout << error << "\n";
    return -1;
  }

  printf("Beatbox started and listening on port %i (sleeping %ims betwween calls).\nType Ctrl+C to stop.\n", OSC_PORT, gSleepy);

  // register signals
  signal(SIGINT,  terminate);

  while (gRun) {
    sleep(1);
  }

  printf("Bye...\n");
}