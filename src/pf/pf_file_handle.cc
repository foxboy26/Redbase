#include "pf_file_handle.h"

#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"
#include "src/pf/pf_internal.h"

PF_FileHandle::PF_FileHandle(PF_BufferPool *bufferPool)
    : bufferPool_(bufferPool), header_(), fd_(-1), isOpen_(false),
      isHeaderModified_(false) {
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

  // seek back to the beginning of the file.
  if (lseek(fd_, 0, SEEK_SET) < 0) {
    return RC::PF_UNIX;
  }
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

  // seek back to the beginning of the file.
  if (lseek(fd_, 0, SEEK_SET) < 0) {
    return RC::PF_UNIX;
  }
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

  if (isHeaderModified_) {
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

RC PF_FileHandle::GetFirstPage(PF_PageHandle *pageHandle) const {
  return GetNextPage(-1, pageHandle);
}

RC PF_FileHandle::GetLastPage(PF_PageHandle *pageHandle) const {
  return GetPrevPage(header_.numPages, pageHandle);
}

RC PF_FileHandle::GetNextPage(PageNum current,
                              PF_PageHandle *pageHandle) const {
  for (int i = current + 1; i < header_.numPages; i++) {
    RC rc = GetThisPage(i, pageHandle);
    if (rc == RC::OK) {
      return RC::OK;
    }
    if (rc == RC::PF_INVALIDPAGE) {
      continue;
    }
  }
  return RC::PF_EOF;
}

RC PF_FileHandle::GetPrevPage(PageNum current,
                              PF_PageHandle *pageHandle) const {
  for (int i = current - 1; i >= 0; i--) {
    RC rc = GetThisPage(i, pageHandle);
    if (rc == RC::OK) {
      return RC::OK;
    }
    if (rc == RC::PF_INVALIDPAGE) {
      continue;
    }
  }

  return RC::PF_EOF;
}

RC PF_FileHandle::GetThisPage(PageNum pageNum,
                              PF_PageHandle *pageHandle) const {
  LOG(INFO) << "GetThisPage...";
  PF_BufferPage *bufferPage;
  RC rc = bufferPool_->GetPage(fd_, pageNum, bufferPage);
  if (rc != RC::OK) {
    return rc;
  }

  // TODO(zhiheng) check header!
  auto *pageHeader = reinterpret_cast<PF_PageHeader *>(bufferPage->Data());
  if (pageHeader->nextFree != USED_PAGE) {
    return RC::PF_INVALIDPAGE;
  }

  *pageHandle =
      PF_PageHandle(pageNum, bufferPage->Data() + PF_PAGE_HEADER_SIZE);

  bufferPage->Pin();

  return RC::OK;
}

RC PF_FileHandle::AllocatePage(PF_PageHandle *pageHandle) {
  LOG(INFO) << "AllocatePage...";
  assert(IsOpen());

  RC rc;
  // allocate a new page.
  if (header_.firstFree == LAST_FREE) {
    PageNum pageNum = header_.numPages;
    rc = AllocateNewPage(pageNum);
    if (rc != RC::OK) {
      return rc;
    }
    rc = GetThisPage(pageNum, pageHandle);
    if (rc != RC::OK) {
      return rc;
    }
    header_.numPages++;
  } else {
    LOG(INFO) << "reuse on free list, firstFree:  " << header_.firstFree;
    // reuse ones on the free list.
    PF_BufferPage *bufferPage;
    PageNum pageNum = header_.firstFree;
    rc = bufferPool_->GetPage(fd_, pageNum, bufferPage);
    if (rc != RC::OK) {
      return rc;
    }
    PF_PageHeader *pageHeader =
        reinterpret_cast<PF_PageHeader *>(bufferPage->Data());
    // Update the nextFree pointer.
    header_.firstFree = pageHeader->nextFree;
    // Mark this page as used.
    pageHeader->nextFree = USED_PAGE;

    *pageHandle =
        PF_PageHandle(pageNum, bufferPage->Data() + PF_PAGE_HEADER_SIZE);

    bufferPage->MarkDirty();
    bufferPage->Pin();
  }

  isHeaderModified_ = true;

  return RC::OK;
}

RC PF_FileHandle::AllocateNewPage(PageNum pageNum) {
  LOG(INFO) << "AllocateNewPage...";
  assert(IsOpen());

  auto bufferPage =
      std::unique_ptr<PF_BufferPage>(new PF_BufferPage(fd_, pageNum));
  // Set page header.
  auto *pageHeader = reinterpret_cast<PF_PageHeader *>(bufferPage->Data());
  pageHeader->nextFree = USED_PAGE;
  bufferPage->MarkDirty();
  RC rc = bufferPage->WriteBack();
  if (rc != RC::OK) {
    return rc;
  }
  return bufferPool_->InsertPage(fd_, pageNum, std::move(bufferPage));
}

RC PF_FileHandle::DisposePage(PageNum pageNum) {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));

  RC rc;
  PF_BufferPage *bufferPage;
  rc = bufferPool_->GetPage(fd_, pageNum, bufferPage);
  if (rc != RC::OK) {
    return rc;
  }

  if (bufferPage->IsPinned()) {
    return RC::PF_PAGEPINNED;
  }

  // update nextFree.
  PF_PageHeader *pageHeader =
      reinterpret_cast<PF_PageHeader *>(bufferPage->Data());
  pageHeader->nextFree = header_.firstFree;
  header_.firstFree = pageNum;

  // clear the data section.
  memset(bufferPage->Data() + PF_PAGE_HEADER_SIZE, 0, PF_PAGE_DATA_SIZE);
  bufferPage->MarkDirty();

  isHeaderModified_ = true;
  return RC::OK;
}

RC PF_FileHandle::MarkDirty(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  return bufferPool_->MarkDirty(fd_, pageNum);
}

RC PF_FileHandle::UnpinPage(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  return bufferPool_->UnpinPage(fd_, pageNum);
}

RC PF_FileHandle::ForcePages(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  if (pageNum == ALL_PAGES) {
    return bufferPool_->ForceAllPages(fd_);
  }
  return bufferPool_->ForcePage(fd_, pageNum);
}
