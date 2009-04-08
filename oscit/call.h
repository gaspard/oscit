#ifndef _OSCIT_CALL_H_
#define _OSCIT_CALL_H_
#include <string>

namespace oscit {
  
/** This is a simple struct to store a call (url and parameter). */
struct Call
{
  Call(const std::string &url, const Value& param) : url_(url), param_(param) {}
  std::string url_;
  Value param_;
};
} // namespace oscit

#endif // _OSCIT_CALL_H_