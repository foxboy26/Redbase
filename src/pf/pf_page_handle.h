#ifndef PF_PAGE_HANDLE_H
#define PF_PAGE_HANDLE_H

#include "src/pf/pf.h"
#include "src/rc.h"

class PF_PageHandle {
public:
  PF_PageHandle(); // Default constructor
  PF_PageHandle(PageNum pageNum, char *pData)
      : pageNum_(pageNum), pData_(pData) {}
  ~PF_PageHandle(); // Destructor

  // non-copyable
  PF_PageHandle(const PF_PageHandle &) = default;
  PF_PageHandle &operator=(const PF_PageHandle &) = default;

  // GetPageNum returns the page number.
  PageNum GetPageNum() const;
  RC GetData(char *&pData) const;

private:
  PageNum pageNum_;
  char *pData_;
};

#endif // PF_PAGE_HANDLE_H
