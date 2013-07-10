#include <exception>
#include <cstdint>
#include <sstream>

/* for test only */
#include <iostream>
using namespace std;

class Exception : public std::exception
{
public:
  virtual const char* what() const throw();
protected:
  std::string PrintMessage(const char* module, const char* message) const;
};

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
