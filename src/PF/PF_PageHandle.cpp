#include "PF_PageHandle.h"

PF_PageHandle::PF_PageHandle()
{
}

PF_PageHandle::~PF_PageHandle()
{
}

PF_PageHandle::PF_PageHandle(const PF_PageHandle& pageHandle)
{
  this->pageNum = pageHandle.pageNum;
  this->pData = pageHandle.pData;
}

PF_PageHandle& PF_PageHandle::operator=(const PF_PageHandle& pageHandle)
{
  if (this != &pageHandle)
  {
    this->pageNum = pageHandle.pageNum;
    this->pData = pageHandle.pData;
  }

  return *this;
}

RC PF_PageHandle::GetData(char*& pData) const
{
  if (this->pData == NULL)
    return PF_PAGEUNPINNED;

  pData = this->pData;

  return OK;
}

RC PF_PageHandle::GetPageNum(PageNum &pageNum) const
{
  if (this->pData == NULL)
    return PF_PAGEUNPINNED;

  pageNum = this->pageNum;

  return OK;
}
