#include "PF_FileHandle.h"

PF_FileHandle::PF_FileHandle()
: fd(-1), isOpen(false), fileHdr(-1, -1), bufferPool(NULL)
{
}

PF_FileHandle::~PF_FileHandle()
{
  // Nothing
}

PF_FileHandle::PF_FileHandle(const PF_FileHandle& fileHandle)
{
  this->fd = fileHandle.fd;
  this->isOpen = false;
  this->fileHdr = fileHandle.fileHdr;
  this->bufferPool = fileHandle.bufferPool;
}

PF_FileHandle& PF_FileHandle::operator= (const PF_FileHandle& fileHandle)
{
  if (this != &fileHandle)
  {
    this->fd = fileHandle.fd;
    this->isOpen = false;
    this->fileHdr = fileHandle.fileHdr;
    this->bufferPool = fileHandle.bufferPool;
  }

  return *this;
}

RC PF_FileHandle::GetFirstPage(PF_PageHandle& pageHandle) const
{
  return GetNextPage(-1, pageHandle);
}

RC PF_FileHandle::GetLastPage(PF_PageHandle& pageHandle) const
{
  return GetPrevPage(fileHdr.numPages, pageHandle);
}

RC PF_FileHandle::GetNextPage(PageNum current, PF_PageHandle& pageHandle) const
{
  if (!isOpen)
    return PF_CLOSEDFILE;

  int rc;
  for (current++, current < fileHdr.numPages; current++)
  {
    rc = GetThisPage(current, pageHandle);

    if (rc == OK)
      return OK;
    
    // error
    if (rc != PF_INVALIDPAGE)
      return rc;
  }

  return PF_EOF;
}

RC PF_FileHandle::GetPrevPage  (PageNum current, PF_PageHandle& pageHandle) const
{
  if (!isOpen)
    return PF_CLOSEDFILE;

  int rc;
  for (current--, current >= 0; current--)
  {
    rc = GetThisPage(current, pageHandle);

    if (rc == OK)
      return OK;
    
    // error
    if (rc != PF_INVALIDPAGE)
      return rc;
  }

  return PF_EOF;
}

RC PF_FileHandle::GetThisPage  (PageNum pageNum, PF_PageHandle& pageHandle) const
{
  if (!isOpen)
    return PF_CLOSEDFILE;
  return OK;
}

RC PF_FileHandle::AllocatePage (PF_PageHandle& pageHandle)
{
  if (!isOpen)
    return PF_CLOSEDFILE;
  return OK;
}

RC PF_FileHandle::DisposePage  (PageNum pageNum)
{
  if (!isOpen)
    return PF_CLOSEDFILE;

  return OK;
}

RC PF_FileHandle::MarkDirty(PageNum pageNum) const
{
  if (!isOpen)
    return PF_CLOSEDFILE;

  return bufferPool->MarkDirty(fd, pageNum);
}

RC PF_FileHandle::UnpinPage(PageNum pageNum) const
{
  if (!isOpen)
    return PF_CLOSEDFILE;

  return bufferPool->UnpinPage(fd, pageNum);
}

RC PF_FileHandle::ForcePages(PageNum pageNum = ALL_PAGES) const
{
  if (!isOpen)
    return PF_CLOSEDFILE;
  
  return bufferPool->ForcePages(pageNum);
}
