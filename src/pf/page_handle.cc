#include "page_handle.h"

namespace redbase {
namespace pf {
PageHandle::PageHandle() : pageNum_(-1), pData_(nullptr) {}

PageHandle::~PageHandle() {}

// PageHandle::PageHandle(const PageHandle& pageHandle)
// {
//   this->pageNum = pageHandle.pageNum;
//   this->pData = pageHandle.pData;
// }
//
// PageHandle& PageHandle::operator=(const PageHandle& pageHandle)
// {
//   if (this != &pageHandle)
//   {
//     this->pageNum = pageHandle.pageNum;
//     this->pData = pageHandle.pData;
//   }
//
//   return *this;
// }

RC PageHandle::GetData(char *&pData) const {
  pData = pData_;
  return RC::OK;
}

PageNum PageHandle::GetPageNum() const { return this->pageNum_; }

} // namespace pf
} // namespace redbase
