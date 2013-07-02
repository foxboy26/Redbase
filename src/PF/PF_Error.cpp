#include "PF.h"
#include "Error.h"

#include <cstdio>
#include <cerrno>

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

class PF_Error : public Error
{
  void PrintError();
};

void PF_Error::PrintError(RC rc)
{
  if (rc >= START_PF_WARN && rc <= END_PF_WARN)
  {
    fprintf(stderr, "[PF] warning: %s.\n", PF_WarningMsg[rc - START_PF_WARN]);
  }
  else if (-rc >= START_PF_ERR && -rc <= END_PF_ERR)
  {
    fprintf(stderr, "[PF] error: %s.\n", PF_ErrorMsg[-rc + START_PF_ERR]);
  }
  else if (rc == PF_UNIX)
  {
    fprintf(stderr, "[PF] error: %s.\n", strerror(errno));
  }
  else if (rc == OK)
  {
    fprintf(stderr, "[PF]: PF_PrintError called with rc = 0.\n");
  }
  else
  {
    fprintf(stderr, "[PF]: Unexpected return code.\n");
  }
}
