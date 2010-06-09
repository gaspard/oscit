// compile with
// g++ -I../include ../build/liboscit.a send.cpp -o send

/** This simple application sends arbitrary values through osc. */

#include <stdio.h>
#include <stdlib.h>  // atoi
#include <iostream>

#include "oscit/oscit.h"

using namespace oscit;


bool gRun = true;

void terminate(int sig) {
  gRun = false;
}

void input(const char *name, std::string &rval) {
  std::string tmp;
  printf("%s %18s: ", name, rval.c_str());
  std::getline(std::cin, tmp);
  if (tmp != "") rval = tmp;
}


class OscCommandShowReplies : public OscCommand {
public:
  OscCommandShowReplies() {}

  virtual void receive(const Url &url, const Value &val) {
    std::cout << "** " << url.path() << " received " << val << std::endl << std::endl;
    Command::receive(url, val);
  }
};


int main(int argc, char * argv[]) {
  Root root;

  std::string url_str("oscit://\"receive\"/message");
  std::string value_str("null");
  std::string continue_input("yes");
  Value value;

  // open osc command.
  OscCommand *sender = root.adopt_command(new OscCommandShowReplies());

  ZeroConfBrowser browser(OSCIT_SRV_TYPE);
  browser.set_command(sender);
  // FIXME: the lines above should be:
  // ZeroConfBrowser *browser = sender->adopt_zeroconf_browser(new ZeroConfBrowser(oscit::SERVICE_TYPE));

  // register signals
  signal(SIGINT,  terminate);

  printf("Send started and listening on port %i.\nValues are parsed as Json\n", sender->port());

  while (gRun) {
    input("Enter   url", url_str);
    input("Enter value", value_str);
    value.set(Json(value_str));
    Url url(url_str);
    // If the location is a service name, resolve
    url.resolve_with(&browser);

    // This is the interesting part: send an osc value out
    sender->send(url.location(), url.path().c_str(), value);

    Thread::millisleep(10); // wait for answer

    input("Continue ? ", continue_input);
    if (continue_input != "yes") {
      gRun = false;
    }
  }

  printf("Bye...\n");
}