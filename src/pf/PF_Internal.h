#ifndef PF_PF_INTERNAL_H
#define PF_PF_INTERNAL_H

#include "PF.h"

static const int PF_BUFFER_SIZE = 40;

static const PageNum ALL_PAGES = -1;

static const PageNum PF_LAST_FREE_PAGE = -1;

static const PageNum USED_PAGE = -2; 

struct PF_FileHeader {
  int numOfPages;
  int firstFree;
};

struct PF_PageHeader {
  PageNum nextFree;
};

static const size_t PF_PAGE_HEADER_SIZE = sizeof(PF_PageHeader);

#endif //  PF_PF_INTERNAL_H
