#ifndef PF_INTERNAL
#define PF_INTERNAL

#include "pf.h"

#include <sys/types.h>

namespace redbase {
namespace pf {
struct PageHeader {
  // PageNum next;
  // PageNum prev;
  PageNum nextFree;
};

constexpr int File_HEADER_SIZE = 1024; // 1kb
constexpr int PAGE_HEADER_SIZE = sizeof(PageHeader);
constexpr int PAGE_SIZE = 4096; // 4kb
constexpr int PAGE_DATA_SIZE = PAGE_SIZE - sizeof(PageHeader);
constexpr int BUFFER_SIZE = 40;

const PageNum USED_PAGE = -2;
const PageNum LAST_FREE = -3;

} // namespace pf
} // namespace redbase
#endif // PF_INTERNAL
