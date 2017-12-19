#include "PF_PageHandle.h"

#include <iostream>
using namespace std;

PF_PageHandle::PF_PageHandle() : pageNum(-1), pData(nullptr)
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

char* PF_PageHandle::GetData() const
{
  if (this->pData == nullptr)
    throw PF_Exception(PF_Exception::PAGE_UNPINNED);

  return this->pData;
}

PageNum PF_PageHandle::GetPageNum() const
{
  if (this->pData == nullptr)
    throw PF_Exception(PF_Exception::PAGE_UNPINNED);

  return this->pageNum;
}
