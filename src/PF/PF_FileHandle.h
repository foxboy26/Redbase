#pragma once

#include "PF.h"

struct PF_FileHdr
{
  int numPages;
  int firstFree;
};

class PF_FileHandle
{
  friend class PF_Manager;
public:
  PF_FileHandle   ();
  ~PF_FileHandle  ();
  PF_FileHandle   (const PF_FileHandle& fileHandle);
  PF_FileHandle&  operator= (const PF_FileHandle& fileHandle);

  RC GetFirstPage (PF_PageHandle& pageHandle) const;
  RC GetLastPage  (PF_PageHandle& pageHandle) const;
  RC GetNextPage  (PageNum current, PF_PageHandle& pageHandle) const;
  RC GetPrevPage  (PageNum current, PF_PageHandle& pageHandle) const;
  RC GetThisPage  (PageNum pageNum, PF_PageHandle& pageHandle) const;
  RC AllocatePage (PF_PageHandle& pageHandle);
  RC DisposePage  (PageNum pageNum);
  RC MarkDirty    (PageNum pageNum) const;
  RC UnpinPage    (PageNum pageNum) const;
  RC ForcePages   (PageNum pageNum = ALL_PAGES) const;
private:
  int fd;
  bool isOpen;
  PF_FileHdr fileHdr;
  BufferPool* bufferPool;
};
