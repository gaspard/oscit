#ifndef OSCIT_INCLUDE_OSCIT_LOCATION_H_
#define OSCIT_INCLUDE_OSCIT_LOCATION_H_
#include <string>
#include <ostream>

#include "oscit/thash.h"

namespace oscit {

class Location
{
public:
  static const unsigned long NO_IP = 0xFFFFFFFF;
  static const uint NO_PORT = 0;

  Location() : reference_by_hostname_(false), ip_(NO_IP), port_(NO_PORT) {}
  Location(const char *protocol, const char *service_name) :
                      protocol_(protocol), name_(service_name),
                      reference_by_hostname_(false), ip_(NO_IP), port_(NO_PORT) {}
  Location(const char *protocol, const char *hostname, uint port) :
                      protocol_(protocol), name_(hostname),
                      reference_by_hostname_(true), ip_(NO_IP), port_(port) {}
  Location(const char *protocol, unsigned long ip, uint port) :
                      protocol_(protocol),
                      reference_by_hostname_(true), ip_(ip), port_(port) {
    set_name_from_ip();
  }

  void clear() {
    protocol_ = "";
    name_ = "";
    reference_by_hostname_ = false;
    ip_   = NO_IP;
    port_ = NO_PORT;
  }

  bool operator!=(const Location &other) {
    return ip_       != other.ip_       ||
           port_     != other.port_     ||
           protocol_ != other.protocol_ ||
           reference_by_hostname_ != other.reference_by_hostname_ ||
           name_     != other.name_;
  }

  bool operator==(const Location &other) {
    return protocol_ == other.protocol_ &&
           (
             (ip_ != Location::NO_IP && ip_ == other.ip_ && port_ == other.port_) ||
             (reference_by_hostname_ == other.reference_by_hostname_ && name_ == other.name_)
           );
  }

private:
  friend std::ostream &operator<<(std::ostream &out_stream, const Location &location);
  friend class Url;
  friend uint hashId<const Location&>(const Location &location);

  void set_name_from_ip();

  /** Protocol used (oscit, http, ...)
   */
  std::string protocol_;

  /** This can contain either a hostname (example.com) or a service
   *  name ("stage camera").
   */
  std::string name_;

  /** This tells us if the name_ contains a hostname or a
   *  service name (true if it is a hostname).
   */
  bool reference_by_hostname_;

  /** Once the location has been resolved, we have an IP.
   */
  unsigned long ip_;

  /** Port of the remote object (set when location is resolved)
   */
  uint port_;
};

// ===== std::string =====
// FIXME: why do we need this ?
// sdbm function: taken from http://www.cse.yorku.ca/~oz/hash.html
template<>
inline uint hashId(const Location &location) {
  if (location.ip_ != Location::NO_IP) {
    return hashId<unsigned long>(location.ip_) + hashId<uint>(location.port_);
  } else {
    // FIXME: hashid name_, protocol_, etc.....
    return hashId<const std::string&>(location.name_);
  }
}


} // oscit

#endif // OSCIT_INCLUDE_OSCIT_LOCATION_H_