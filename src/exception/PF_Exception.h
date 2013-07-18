#pragma once

#include "Exception.h"

class PF_Exception : public Exception
{
  public:
    enum RC : std::int8_t 
    {
      PAGEPINNED,
      PAGENOTINBUF,
      PAGEUNPINNED,
      PAGEFREE,
      INVALIDPAGE,
      FILEOPEN,
      CLOSEDFILE,
      INCOMPLETEREAD,
      INCOMPLETEWRITE,
      NOTIMPLEMENTED
    };

    PF_Exception(PF_Exception::RC _rc) : rc(_rc) {}

    const char* what() const throw();

  private:
    static constexpr const char* module = "PF";
    static constexpr const char* message[] = 
    {
      "page pinned in buffer",
      "page to be unpinned is not in buffer",
      "page already unpinned",
      "page already free",
      "invalid page number",
      "file handle already open",
      "file is closed",
      "incomplete read of page from file",
      "incomplete write of page to file",
      "not implemented"
    };

    RC rc;
};
