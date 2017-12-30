#include "pf_file_handle.h"

#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"
#include "src/pf/pf_internal.h"

struct PF_BufferPage {
  PageNum pageNum;
  bool isDirty;
  int pinCount;
};

PF_FileHandle::PF_FileHandle()
    : bufferPool_(nullptr), header_(), fd_(-1), isOpen_(false),
      isHeadModfied_(false) {
  // Nothing
}

PF_FileHandle::~PF_FileHandle() {
  if (IsOpen()) {
    RC rc = Close();
    if (rc != RC::OK) {
      LOG(ERROR) << "Failed to close file; fd=" << fd_ << " err: " << rc;
    }
  }
}

RC PF_FileHandle::ReadFileHeader() {
  assert(IsOpen());
  ssize_t readBytes = read(fd_, &header_, sizeof(header_));

  if (readBytes < 0) {
    return RC::PF_UNIX;
  }

  if (readBytes < static_cast<ssize_t>(sizeof(header_))) {
    return RC::PF_HDRREAD;
  }

  return OK;
}

RC PF_FileHandle::WriteFileHeader() {
  assert(IsOpen());
  ssize_t writeBytes = write(fd_, &header_, sizeof(header_));

  if (writeBytes < 0) {
    return RC::PF_UNIX;
  }

  if (writeBytes < static_cast<ssize_t>(sizeof(header_))) {
    return RC::PF_HDRWRITE;
  }

  return OK;
}

RC PF_FileHandle::Open(const char *filename) {
  // make sure the file is not opened.
  assert(!IsOpen());

  int fd = open(filename, O_RDWR, 0600);
  LOG(INFO) << "fd:  " << fd << "\n";
  if (fd < 0) {
    return RC::PF_UNIX;
  }

  // mark file as opened.
  fd_ = fd;
  isOpen_ = true;

  return ReadFileHeader();
}

RC PF_FileHandle::Close() {
  assert(IsOpen());

  if (isHeadModfied_) {
    RC rc = WriteFileHeader();
    if (rc != RC::OK) {
      return rc;
    }
  }

  if (close(fd_) == -1) {
    return RC::PF_UNIX;
  }

  // Update internal states
  isOpen_ = false;
  fd_ = -1;

  return RC::OK;
}

/*
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
}*/
