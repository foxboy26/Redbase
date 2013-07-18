#pragma once

#include "PF.h"

class PF_PageHandle
{
  public:
    PF_PageHandle();
    ~PF_PageHandle();
    PF_PageHandle(const PF_PageHandle& pageHandle);
    PF_PageHandle& operator= (const PF_PageHandle& pageHandle);

    char* GetData() const;
    PageNum GetPageNum() const;
  private:
    PageNum pageNum;
    char* pData;

  friend class PF_FileHandle;
};
