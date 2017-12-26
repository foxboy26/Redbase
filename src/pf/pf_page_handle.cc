#include "pf_page_handle.h"

PF_PageHandle::PF_PageHandle() : pageNum_(-1), pData_(nullptr) {}

PF_PageHandle::~PF_PageHandle()
{
}

// PF_PageHandle::PF_PageHandle(const PF_PageHandle& pageHandle)
// {
//   this->pageNum = pageHandle.pageNum;
//   this->pData = pageHandle.pData;
// }
// 
// PF_PageHandle& PF_PageHandle::operator=(const PF_PageHandle& pageHandle)
// {
//   if (this != &pageHandle)
//   {
//     this->pageNum = pageHandle.pageNum;
//     this->pData = pageHandle.pData;
//   }
// 
//   return *this;
// }

RC PF_PageHandle::GetData(char *&pData) const
{
  return RC::OK;
}

PageNum PF_PageHandle::GetPageNum() const
{
  return this->pageNum_;
}
