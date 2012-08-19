#pragma once

#include "PF.h"

class PF_PageHandle
{
  friend class PF_FileHandle;
public:
  PF_PageHandle  ();
  ~PF_PageHandle ();
  PF_PageHandle  (const PF_FileHandle& fileHandle);
  PF_PageHandle& operator= (const PF_FileHandle& fileHandle);

  RC GetData     (char*& pData) const;

  RC GetPageNum  (PageNum &pageNum) const;
private:
  PageNum pageNum_;
  char* pData_;
};
