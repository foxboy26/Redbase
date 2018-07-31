#ifndef PF_PAGE_HANDLE_H
#define PF_PAGE_HANDLE_H

#include "src/pf/pf.h"
#include "src/rc.h"

namespace redbase {
namespace pf {
class PageHandle {
public:
  PageHandle(); // Default constructor
  PageHandle(PageNum pageNum, char *pData) : pageNum_(pageNum), pData_(pData) {}
  ~PageHandle(); // Destructor

  // non-copyable
  PageHandle(const PageHandle &) = default;
  PageHandle &operator=(const PageHandle &) = default;

  // GetPageNum returns the page number.
  PageNum GetPageNum() const;
  RC GetData(char *&pData) const;

private:
  PageNum pageNum_;
  char *pData_; // not owned
};

} // namespace pf
} // namespace redbase

#endif // PF_PAGE_HANDLE_H
