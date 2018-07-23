#include "file_handle.h"

#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"

namespace redbase {
namespace pf {
FileHandle::FileHandle(BufferPool *bufferPool)
    : bufferPool_(bufferPool), header_(), fd_(-1), isOpen_(false),
      isHeaderModified_(false) {
  // Nothing
}

FileHandle::~FileHandle() {
  if (IsOpen()) {
    RC rc = Close();
    if (rc != RC::OK) {
      LOG(ERROR) << "Failed to close file; fd=" << fd_ << " err: " << rc;
    }
  }
}

RC FileHandle::ReadFileHeader() {
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

RC FileHandle::WriteFileHeader() {
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

RC FileHandle::Open(const char *filename) {
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

RC FileHandle::Close() {
  assert(IsOpen());

  if (isHeaderModified_) {
    RC rc = WriteFileHeader();
    if (rc != RC::OK) {
      return rc;
    }
  }

  RC rc = bufferPool_->ForceAllPages(fd_);
  if (rc != RC::OK) {
    return rc;
  }

  if (close(fd_) == -1) {
    return RC::PF_UNIX;
  }

  // Update internal states
  isOpen_ = false;
  fd_ = -1;

  return RC::OK;
}

RC FileHandle::GetFirstPage(PageHandle *pageHandle) const {
  return GetNextPage(-1, pageHandle);
}

RC FileHandle::GetLastPage(PageHandle *pageHandle) const {
  return GetPrevPage(header_.numPages, pageHandle);
}

RC FileHandle::GetNextPage(PageNum current, PageHandle *pageHandle) const {
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

RC FileHandle::GetPrevPage(PageNum current, PageHandle *pageHandle) const {
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

RC FileHandle::GetThisPage(PageNum pageNum, PageHandle *pageHandle) const {
  LOG(INFO) << "GetThisPage...";
  BufferPage *bufferPage;
  RC rc = bufferPool_->GetPage(fd_, pageNum, bufferPage);
  if (rc != RC::OK) {
    return rc;
  }

  // TODO(zhiheng) check header!
  auto *pageHeader = reinterpret_cast<PageHeader *>(bufferPage->Data());
  if (pageHeader->nextFree != USED_PAGE) {
    return RC::PF_INVALIDPAGE;
  }

  *pageHandle = PageHandle(pageNum, bufferPage->Data() + PAGE_HEADER_SIZE);

  bufferPage->Pin();

  return RC::OK;
}

RC FileHandle::AllocatePage(PageHandle *pageHandle) {
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
    BufferPage *bufferPage;
    PageNum pageNum = header_.firstFree;
    rc = bufferPool_->GetPage(fd_, pageNum, bufferPage);
    if (rc != RC::OK) {
      return rc;
    }
    PageHeader *pageHeader = reinterpret_cast<PageHeader *>(bufferPage->Data());
    // Update the nextFree pointer.
    header_.firstFree = pageHeader->nextFree;
    // Mark this page as used.
    pageHeader->nextFree = USED_PAGE;

    *pageHandle = PageHandle(pageNum, bufferPage->Data() + PAGE_HEADER_SIZE);

    bufferPage->MarkDirty();
    bufferPage->Pin();
  }

  isHeaderModified_ = true;

  return RC::OK;
}

RC FileHandle::AllocateNewPage(PageNum pageNum) {
  LOG(INFO) << "AllocateNewPage...";
  assert(IsOpen());

  auto bufferPage = std::unique_ptr<BufferPage>(new BufferPage(fd_, pageNum));
  // Set page header.
  auto *pageHeader = reinterpret_cast<PageHeader *>(bufferPage->Data());
  pageHeader->nextFree = USED_PAGE;
  bufferPage->MarkDirty();
  RC rc = bufferPage->WriteBack();
  if (rc != RC::OK) {
    return rc;
  }
  return bufferPool_->InsertPage(fd_, pageNum, std::move(bufferPage));
}

RC FileHandle::DisposePage(PageNum pageNum) {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));

  RC rc;
  BufferPage *bufferPage;
  rc = bufferPool_->GetPage(fd_, pageNum, bufferPage);
  if (rc != RC::OK) {
    return rc;
  }

  if (bufferPage->IsPinned()) {
    return RC::PF_PAGEPINNED;
  }

  // update nextFree.
  PageHeader *pageHeader = reinterpret_cast<PageHeader *>(bufferPage->Data());
  pageHeader->nextFree = header_.firstFree;
  header_.firstFree = pageNum;

  // clear the data section.
  memset(bufferPage->Data() + PAGE_HEADER_SIZE, 0, PAGE_DATA_SIZE);
  bufferPage->MarkDirty();

  isHeaderModified_ = true;
  return RC::OK;
}

RC FileHandle::MarkDirty(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  return bufferPool_->MarkDirty(fd_, pageNum);
}

RC FileHandle::UnpinPage(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  return bufferPool_->UnpinPage(fd_, pageNum);
}

RC FileHandle::ForcePages(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  if (pageNum == ALL_PAGES) {
    return bufferPool_->ForceAllPages(fd_);
  }
  return bufferPool_->ForcePage(fd_, pageNum);
}
} // namespace pf
} // namespace redbase
