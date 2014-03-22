#ifndef PF_PF_H
#define PF_PF_H

#include "PF_Exception.h"

typedef int PageNum;

//static const int PF_PAGE_SIZE = 4092;
static const size_t PF_PAGE_SIZE = 4092 + 4;
//static const size_t PF_PAGE_SIZE = 16;

class PF_Manager;

class PF_FileHandle;

class PF_PageHandle;

#endif // PF_PF_H
