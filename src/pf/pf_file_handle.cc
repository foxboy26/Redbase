#include "pf_file_handle.h"

const int PF_PAGE_SIZE = 4092; // 4Kb
const int PF_BUFFER_SIZE = 40;

struct PF_BufferPage {
  PageNum pageNum;
  bool isDirty;
  int pinCount;
};

PF_FileHandle::PF_FileHandle()
    : fd(-1), isOpen(false), isHeadModified(false), bufferPool(nullptr) {
  // Nothing
}

PF_FileHandle::~PF_FileHandle() {
  // Nothing
}

PF_PageHandle PF_FileHandle::GetFirstPage() const { return GetNextPage(-1); }

PF_PageHandle PF_FileHandle::GetNextPage(PageNum current) const {
  // assert(isOpen);
  // assert(isValidPageNum(current));

  PF_PageHandle pageHandle;

  for (current--; current >= 0; current--) {
    char *pBufData = bufferPool->GetPage(this->fd, current);

    PF_PageHeader *pageHeader = reinterpret_cast<PF_PageHeader *>(pBufData);

    if (pageHeader->nextFree == USED_PAGE) {
      pageHandle.pageNum = current;
      pageHandle.pData = pBufData + PF_PAGE_HEADER_SIZE;
      return pageHandle;
    }
  }

  throw PF_Exception(PF_Exception::RC::END_OF_FILE);
}

PF_PageHandle PF_FileHandle::GetLastPage() const {
  throw PF_Exception(PF_Exception::RC::NOT_IMPLEMENTED);
}

PF_PageHandle PF_FileHandle::GetPrevPage(PageNum current) const {
  throw PF_Exception(PF_Exception::RC::NOT_IMPLEMENTED);
}

PF_PageHandle PF_FileHandle::GetPage(PageNum pageNum) const {
  // assert(isOpen);
  // assert(isValidPageNum(pageNum));

  char *pBufData = bufferPool->GetPage(this->fd, pageNum);

  PF_PageHeader *pageHeader = reinterpret_cast<PF_PageHeader *>(pBufData);

  if (pageHeader->nextFree != USED_PAGE) {

    bufferPool->UnpinPage(this->fd, pageNum);
    throw PF_Exception(PF_Exception::RC::INVALID_PAGE);
  }

  PF_PageHandle pageHandle;
  pageHandle.pageNum = pageNum;
  pageHandle.pData = pBufData + PF_PAGE_HEADER_SIZE;

  return pageHandle;
}

PF_PageHandle PF_FileHandle::AllocatePage() {
  // assert(isOpen);

  char *pBufData;
  int pageNum;
  PF_PageHeader *pageHeader;

  if (fileHeader.firstFree != PF_LAST_FREE_PAGE) {
    pageNum = fileHeader.firstFree;

    pBufData = bufferPool->GetPage(this->fd, pageNum);

    pageHeader = reinterpret_cast<PF_PageHeader *>(pBufData);
    pageHeader->nextFree = USED_PAGE;

    fileHeader.firstFree = pageHeader->nextFree;

  } else {
    pageNum = fileHeader.numOfPages;

    pBufData = bufferPool->AllocatePage(this->fd, pageNum);

    pageHeader = reinterpret_cast<PF_PageHeader *>(pBufData);
    pageHeader->nextFree = USED_PAGE;

    fileHeader.numOfPages++;
  }

  this->isHeadModified = true;

  bufferPool->MarkDirty(this->fd, pageNum);

  PF_PageHandle pageHandle;
  pageHandle.pageNum = pageNum;
  pageHandle.pData = pBufData + PF_PAGE_HEADER_SIZE;

  return pageHandle;
}

void PF_FileHandle::DisposePage(PageNum pageNum) {
  // assert(isOpen);
  // assert(isValidPageNum(pageNum));

  char *pBufData = this->bufferPool->GetPage(this->fd, pageNum);

  PF_PageHeader *pageHeader = reinterpret_cast<PF_PageHeader *>(pBufData);

  if (pageHeader->nextFree != USED_PAGE) {
    bufferPool->UnpinPage(this->fd, pageNum);
    throw PF_Exception(PF_Exception::RC::PAGE_FREE);
  }

  pageHeader->nextFree = this->fileHeader.firstFree;

  this->fileHeader.firstFree = pageNum;

  bufferPool->UnpinPage(this->fd, pageNum);

  bufferPool->MarkDirty(this->fd, pageNum);

  this->isHeadModified = true;
}

void PF_FileHandle::MarkDirty(PageNum pageNum) const {
  // assert(isOpen);
  // assert(isValidPageNum(pageNum));

  bufferPool->MarkDirty(fd, pageNum);
}

void PF_FileHandle::UnpinPage(PageNum pageNum) const {
  // assert(isOpen);
  // assert(isValidPageNum(pageNum));

  bufferPool->UnpinPage(fd, pageNum);
}

void PF_FileHandle::ForcePages(PageNum pageNum) const {
  // assert(isOpen);
  // assert(isValidPageNum(pageNum));

  bufferPool->ForcePages(pageNum);
}

inline bool PF_FileHandle::isValidPageNum(const PageNum &pageNum) const {
  return (pageNum >= 0 && pageNum <= fileHeader.numOfPages);
}
