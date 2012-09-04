#pragma once

#include "RedBase.h"

typedef int PageNum;

//static const int PF_PAGE_SIZE = 4092;
static const int PF_PAGE_SIZE = 4092 + 4;

static const int PF_BUFFER_SIZE = 40;

// Warning code starts from START_PF_WARN = 1
static const RC PF_EOF             = START_PF_WARN + 0; // end of file
static const RC PF_PAGEPINNED      = START_PF_WARN + 1; // page pinned in buffer
static const RC PF_PAGENOTINBUF    = START_PF_WARN + 2; // page to be unpinned is not in buffer
static const RC PF_PAGEUNPINNED    = START_PF_WARN + 3; // page already unpinned
static const RC PF_PAGEFREE        = START_PF_WARN + 4; // page already free
static const RC PF_INVALIDPAGE     = START_PF_WARN + 5; // invalid page number
static const RC PF_FILEOPEN        = START_PF_WARN + 6; // file handle already open
static const RC PF_CLOSEDFILE      = START_PF_WARN + 7; // file is closed
static const RC END_PF_WARN        = PF_CLOSEDFILE;

// Error code starts from START_PF_ERR = -1
static const RC PF_NOMEM           = START_PF_ERR - 0;  // no memory
static const RC PF_NOBUF           = START_PF_ERR - 1;  // no buffer space
static const RC PF_INCOMPLETEREAD  = START_PF_ERR - 2;  // incomplete read from file
static const RC PF_INCOMPLETEWRITE = START_PF_ERR - 3;  // incomplete write to file
static const RC PF_HDRREAD         = START_PF_ERR - 4;  // incomplete read of header
static const RC PF_HDRWRITE        = START_PF_ERR - 5;  // incomplete write to header
// Internal errors
static const RC PF_PAGEINBUF       = START_PF_ERR - 6;  // new page already in buffer
static const RC PF_HASHNOTFOUND    = START_PF_ERR - 7;  // hash table entry not found
static const RC PF_HASHPAGEEXIST   = START_PF_ERR - 8;  // page already in hash table
static const RC PF_INVALIDNAME     = START_PF_ERR - 9;  // invalid PC file name
// Error in UNIX system call or library routine
static const RC PF_UNIX            = START_PF_ERR - 10; // unix error
static const RC END_PF_ERROR       = PF_UNIX;

void PF_PrintError(RC rc);

class PF_Manager;

class PF_FileHandle;

class PF_PageHandle;
