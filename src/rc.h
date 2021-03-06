#ifndef RC_H
#define RC_H

namespace redbase {
enum RC {
  OK = 0, // success

  PF_EOF,          // end of file
  PF_PAGEPINNED,   // page pinned in buffer
  PF_PAGENOTINBUF, // page to be unpinned is not in buffer
  PF_PAGEUNPINNED, // page already unpinned
  PF_PAGEFREE,     // page already free
  PF_INVALIDPAGE,  // invalid page number
  PF_FILEOPEN,     // file handle already open
  PF_CLOSEDFILE,   // file is closed

  PF_NOMEM,           // out of memory
  PF_NOBUF,           // out of buffer space
  PF_INCOMPLETEREAD,  // incomplete read of page from file
  PF_INCOMPLETEWRITE, // incomplete write of page to file
  PF_HDRREAD,         // incomplete read of header from file
  PF_HDRWRITE,        // incomplete write of header to file
  // Internal PF errors:
  PF_PAGEINBUF,     // new allocated page already in buffer
  PF_HASHNOTFOUND,  // hash table entry not found
  PF_HASHPAGEEXIST, // page already exists in hash table
  PF_INVALIDNAME,   // invalid file name
  PF_UNIX,          // Unix error

  RM_INVALID_RECORD,    // Invalid record indicates no data or rid is invalid.
  RM_INVALIDRECORDSIZE, //
  RM_RECORDNOTFOUND,    //

  NOT_IMPLEMENTED, // function not implemented.

  RC_EOF,
};

const char *const RC_Name[] = {
    "OK", // success

    "PF_EOF",          // end of file
    "PF_PAGEPINNED",   // page pinned in buffer
    "PF_PAGENOTINBUF", // page to be unpinned is not in buffer
    "PF_PAGEUNPINNED", // page already unpinned
    "PF_PAGEFREE",     // page already free
    "PF_INVALIDPAGE",  // invalid page number
    "PF_FILEOPEN",     // file handle already open
    "PF_CLOSEDFILE",   // file is closed

    "PF_NOMEM",           // out of memory
    "PF_NOBUF",           // out of buffer space
    "PF_INCOMPLETEREAD",  // incomplete read of page from file
    "PF_INCOMPLETEWRITE", // incomplete write of page to file
    "PF_HDRREAD",         // incomplete read of header from file
    "PF_HDRWRITE",        // incomplete write of header to file
    // Internal PF errors:
    "PF_PAGEINBUF",     // new allocated page already in buffer
    "PF_HASHNOTFOUND",  // hash table entry not found
    "PF_HASHPAGEEXIST", // page already exists in hash table
    "PF_INVALIDNAME",   // invalid file name
    "PF_UNIX",          // Unix error

    "NOT_IMPLEMENTED", // function not implemented
};
} // namespace redbase

#endif // RC_H
