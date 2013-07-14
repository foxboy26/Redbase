#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
  public:
    virtual const char* what() const throw();
  protected:
    std::string PrintMessage(const char* module, const char* message) const;
};
