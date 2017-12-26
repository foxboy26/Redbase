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

  RC_EOF,
};

void PF_PrintError(RC rc);
