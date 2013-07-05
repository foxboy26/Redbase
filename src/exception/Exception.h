#include <exception>
#include <cstdint>

class Exception : public std::exception
{
  virtual const char* what() const throw();
};

const char* Exception::what() const throw()
{
  return "General exception";
}
