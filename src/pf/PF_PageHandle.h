#pragma once

#include "PF.h"

class PF_PageHandle
{
  friend class PF_PageHandle;
public:
  PF_PageHandle  ();
  ~PF_PageHandle ();
  PF_PageHandle  (const PF_PageHandle& pageHandle);
  PF_PageHandle& operator= (const PF_PageHandle& pageHandle);

  RC GetData     (char*& pData) const;

  RC GetPageNum  (PageNum &pageNum) const;
private:
  PageNum pageNum;
  char* pData;
};
