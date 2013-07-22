#include "PF_Exception.h"

#include <errno.h>
#include <string.h>

constexpr const char* PF_Exception::module;
constexpr const char* PF_Exception::message[];

const char* PF_Exception::what() const throw()
{
  if (rc == UNIX)
    return PrintMessage(module, strerror(errno)).c_str();
  else
    return PrintMessage(module, message[static_cast<int> (rc)]).c_str();
}
