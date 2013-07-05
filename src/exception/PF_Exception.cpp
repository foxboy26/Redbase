#include "Exception.h"

enum class PF_RC : std::int8_t 
{
  EOF,
  ABC
};

static const char* const PF_WarningMsg[] = 
{
  "end of file",
  "page pinned in buffer",
  "page to be unpinned is not in buffer",
  "page already unpinned",
  "page already free",
  "invalid page number",
  "file handle already open",
  "file is closed"
};

static const char* const PF_ErrorMsg[] = 
{
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

class PF_Exception : public Exception
{
public:
  PF_Exception(PF_RC rc) : rc(rc);

  const char* what() const throw()
  {
    return message[static_cast<int>(rc)];
  }

private:
  PF_RC rc;
  static const char* const message[];
}
