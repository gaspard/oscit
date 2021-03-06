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

#import <Foundation/NSNetServices.h>
#import <Foundation/Foundation.h>

#include "oscit/zeroconf_browser.h"
#include "oscit/proxy_factory.h"
#include "oscit/root_proxy.h"
#include "oscit/object_proxy.h"
#include "oscit/location.h"

using namespace oscit;

@interface NetServiceDelegate : NSObject {
  oscit::ZeroConfBrowser *browser_;
  NSNetServiceBrowser *service_browser_;
  std::string protocol_;
  std::string service_type_;
}

- (id)initWithDelegate:(oscit::ZeroConfBrowser *)master serviceType:(const char *)service_type protocol:(const char *)protocol;
- (void)stop;
- (void)start;
// NSNetServiceBrowser delegate methods
- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)browser;
- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)browser;
- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didNotSearch:(NSDictionary *)errorDict;
- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing;
- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing;

// resolving host
- (void)netServiceDidResolveAddress:(NSNetService *)service;
- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict;
@end

@implementation NetServiceDelegate
- (id)initWithDelegate:(oscit::ZeroConfBrowser *)master
        serviceType:(const char *)service_type
        protocol:(const char *)protocol {
  if (self = [super init]) {
    protocol_ = protocol;
    browser_   = master;
    service_browser_  = [[NSNetServiceBrowser alloc] init];
    service_type_ = service_type;
    [service_browser_ setDelegate:self];
  }
  return self;
}

- (void)start {
  if (!browser_->is_running()) {
    [service_browser_ stop];
    [service_browser_ searchForServicesOfType:[NSString stringWithCString:service_type_.c_str() encoding:NSUTF8StringEncoding] inDomain:@""];
    browser_->set_running(true);
  }
}

- (void)stop {
  if (browser_->is_running()) {
    [service_browser_ stop];
    browser_->set_running(false);
  }
}

// NSNetServiceBrowser delegate methods
- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)browser {
  // ignore
}

- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)browser {
  // ignore
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didNotSearch:(NSDictionary *)errorDict {
  // handle errors ....
}

- (void)netServiceWillResolve:(NSNetService *)service {
  NSLog(@"Going to resolve %@\n", [service name]);
}

- (void)netServiceDidResolveAddress:(NSNetService *)service {
  NSLog(@"\n\nResolved: %@ in %@:%i addresses: %i\n", [service name], [service hostName], [service port], [[service addresses] count]);

  NSLog(@"master: %p\n", browser_);
  browser_->add_device(Location(
                          protocol_.c_str(),
                          [[service name] UTF8String],
                          [[service hostName] UTF8String],
                          [service port]
                          ));

  NSLog(@"device added.\n");
  [service release];
}

- (void)netService:(NSNetService *)service didNotResolve:(NSDictionary *)errorDict {
  NSLog(@"Could not resolve %@!\n", [service name]);
  // handle errors ...
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didFindService:(NSNetService *)service moreComing:(BOOL)moreComing {
  NSLog(@"\n\nFound: %@ in %@:%i addresses: %i\n", [service name], [service hostName], [service port], [[service addresses] count]);
  [service retain];
  [service setDelegate:self];
  [service resolveWithTimeout:0.0];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
  browser_->remove_device( [[aNetService name] UTF8String]);
}

-(BOOL) respondsToSelector:(SEL)selector {
  NSLog(@"SELECTOR: %@\n", NSStringFromSelector(selector));
  return [super respondsToSelector:selector];
}

-(void)dealloc {
  [self stop];
  [service_browser_ release];
  [super dealloc];
}

@end

namespace oscit {

class ZeroConfBrowser::Implementation {
public:
  Implementation(ZeroConfBrowser *master) {
    delegate_ = [[NetServiceDelegate alloc] initWithDelegate:master serviceType:master->service_type_.c_str() protocol:master->protocol_.c_str()];
  }

  ~Implementation() {
    [delegate_ release];
  }

  void start() {
    [delegate_ start];
  }

  void stop() {
    [delegate_ stop];
  }

  NetServiceDelegate *delegate_;
};

ZeroConfBrowser::ZeroConfBrowser(const char *service_type) :
                  service_type_(service_type),
                  command_(NULL),
                  proxy_factory_(NULL),
                  found_devices_(FOUND_DEVICE_HASH_SIZE) {
  get_protocol_from_service_type();
  NSLog(@"master: %p\n", this);
  impl_ = new ZeroConfBrowser::Implementation(this);
}

ZeroConfBrowser::~ZeroConfBrowser() {
  delete impl_;
  if (proxy_factory_) delete proxy_factory_;
}

void ZeroConfBrowser::start() {
  impl_->start();
}

void ZeroConfBrowser::stop() {
  impl_->stop();
}

} // oscit



