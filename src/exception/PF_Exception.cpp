#include "PF_Exception.h"

constexpr const char* PF_Exception::module;
constexpr const char* PF_Exception::message[];

const char* PF_Exception::what() const throw()
{
  return PrintMessage(module, message[static_cast<int> (rc)]).c_str();
}
