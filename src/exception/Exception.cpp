#include "Exception.h"

#include <sstream>

const char* Exception::what() const throw()
{
  return "General exception";
}

std::string Exception::PrintMessage(const char* module, const char* message) const
{
  std::stringstream ss;
  ss << "Exception: [" << module << "] " << message;
  return ss.str();
}
