#include "Exception.h"

#include <iostream>

class PF_Exception : public Exception
{
public:
  enum class RC : std::int8_t 
  {
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
  "end of file",
  "page pinned in buffer",
  "page to be unpinned is not in buffer",
  "page already unpinned",
  "page already free",
  "invalid page number",
  "file handle already open",
  "file is closed",
  "no memory",
  "no buffer space",
  "incomplete read of page from file",
  "incomplete write of page to file",
  "incomplete read of header from file",
  "incomplete write of header from file",
  "new page to be allocated already in buffer",
  "hash table entry not found",
  "page already in hash table",
  "invalid file name"
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
