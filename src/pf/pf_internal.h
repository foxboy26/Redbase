#ifndef PF_PF_INTERNAL
#define PF_PF_INTERNAL

#include "pf.h"

#include <sys/types.h>

struct PF_PageHeader {
  // PageNum next;
  // PageNum prev;
  PageNum nextFree;
};

constexpr int PF_File_HEADER_SIZE = 1024; // 1kb
constexpr int PF_PAGE_HEADER_SIZE = sizeof(PF_PageHeader);
constexpr int PF_PAGE_SIZE = 4096; // 4kb
constexpr int PF_PAGE_DATA_SIZE = PF_PAGE_SIZE - sizeof(PF_PageHeader);
constexpr int PF_BUFFER_SIZE = 40;

const PageNum USED_PAGE = -2;
const PageNum LAST_FREE = -3;

off_t PageOffset(PageNum pageNum) {
  return PF_File_HEADER_SIZE + pageNum * PF_PAGE_SIZE;
}

#endif // PF_PF_INTERNAL
