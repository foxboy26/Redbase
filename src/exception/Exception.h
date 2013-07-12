#pragma once

#include <exception>
#include <sstream>

class Exception : public std::exception
{
  public:
    virtual const char* what() const throw();
  protected:
    std::string PrintMessage(const char* module, const char* message) const
    {
      std::stringstream ss;
      ss << "Exception: [" << module << "] " << message;
      return ss.str();
    }
};

const char* Exception::what() const throw()
{
  return "General exception";
}
