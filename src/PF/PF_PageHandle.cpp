#include "PF_PageHandle.h"

PF_PageHandle::PF_PageHandle()
: pageNum_(-1), pData_(NULL)
{
}

PF_PageHandle::~PF_PageHandle()
{
}

PF_PageHandle::PF_PageHandle(const PF_FileHandle& fileHandle)
{
  this->pageNum_ = fileHandle.pageNum_;
  this->pData_ = fileHandle.pData_;
}

PF_PageHandle& PF_PageHandle::operator=(const PF_FileHandle& fileHandle)
{
  if (this != &fileHandle)
  {
    this->pageNum_ = fileHandle.pageNum_;
    this->pData_ = fileHandle.pData_;
  }

  return (*this);
}

RC PF_PageHandle::GetData(char*& pData) const
{
  if (this->pData_ == NULL)
    return PF_PAGEUNPINNED;

  pData = this->pData_;

  return OK;
}

RC PF_PageHandle::GetPageNum(PageNum &pageNum) const
{
  if (this->pData_ == NULL)
    return PF_PAGEUNPINNED;

  pageNum = this->pageNum_;

  return OK;
}
