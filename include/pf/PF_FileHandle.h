#ifndef PF_PF_FILEHANDLE_H
#define PF_PF_FILEHANDLE_H

#include "PF.h"
#include "PF_Internal.h"
#include "PF_BufferPool.h"
#include "PF_PageHandle.h"

class PF_FileHandle
{
  friend class PF_Manager;
public:
  PF_FileHandle   ();
  ~PF_FileHandle  ();
  PF_FileHandle   (const PF_FileHandle& fileHandle);
  PF_FileHandle&  operator= (const PF_FileHandle& fileHandle);

  PF_PageHandle GetFirstPage () const;
  PF_PageHandle GetLastPage  () const;
  PF_PageHandle GetNextPage  (PageNum current) const;
  PF_PageHandle GetPrevPage  (PageNum current) const;
  PF_PageHandle GetPage  (PageNum pageNum) const;
  PF_PageHandle AllocatePage ();
  void DisposePage  (PageNum pageNum);
  void MarkDirty    (PageNum pageNum) const;
  void UnpinPage    (PageNum pageNum) const;
  void ForcePages   (PageNum pageNum = ALL_PAGES) const;
private:
  int fd;
  bool isOpen;
  bool isHeadModified;
  PF_FileHeader fileHeader;
  PF_BufferPool* bufferPool;

  bool isValidPageNum(const PageNum& pageNum) const;
};

#endif // PF_PF_FILEHANDLE_H
