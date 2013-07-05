#include "Exception.h"

#include <iostream>

class PF_Exception : public Exception
{
public:
  enum class RC : std::int8_t 
  {
    aaa,
    ABC
  };

  PF_Exception(PF_Exception::RC rc) : rc(rc) {}

  const char* what() const throw();

private:
  RC rc;
  static const char* const message[];
};

const char* const PF_Exception::message[] = 
{
  "eof",
  "abc"
};

const char* PF_Exception::what() const throw()
{
  return message[static_cast<int>(rc)];
}

int main()
{
  try {
    throw PF_Exception(PF_Exception::RC::aaa);
  } catch (PF_Exception e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
