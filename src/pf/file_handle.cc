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
    : buffer_pool_(bufferPool), header_(), fd_(-1), is_open_(false),
      is_header_modified(false) {
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
  ssize_t read_bytes = read(fd_, &header_, sizeof(header_));
  if (read_bytes < 0) {
    return RC::PF_UNIX;
  }
  if (read_bytes < static_cast<ssize_t>(sizeof(header_))) {
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

RC FileHandle::Open(absl::string_view file_name) {
  // make sure the file is not opened.
  assert(!IsOpen());

  int fd = open(std::string(file_name).c_str(), O_RDWR, 0600);
  LOG(INFO) << "fd:  " << fd << "\n";
  if (fd < 0) {
    return RC::PF_UNIX;
  }

  // mark file as opened.
  fd_ = fd;
  is_open_ = true;

  return ReadFileHeader();
}

RC FileHandle::Close() {
  assert(IsOpen());

  if (is_header_modified) {
    RC rc = WriteFileHeader();
    if (rc != RC::OK) {
      return rc;
    }
  }

  RC rc = buffer_pool_->ForceAllPages(fd_);
  if (rc != RC::OK) {
    return rc;
  }

  if (close(fd_) == -1) {
    return RC::PF_UNIX;
  }

  // Update internal states
  is_open_ = false;
  fd_ = -1;

  return RC::OK;
}

RC FileHandle::GetFirstPage(PageHandle *page_handle) const {
  return GetNextPage(-1, page_handle);
}

RC FileHandle::GetLastPage(PageHandle *page_handle) const {
  return GetPrevPage(header_.numPages, page_handle);
}

RC FileHandle::GetNextPage(PageNum current, PageHandle *page_handle) const {
  for (int i = current + 1; i < header_.numPages; i++) {
    RC rc = GetThisPage(i, page_handle);
    if (rc == RC::OK) {
      return RC::OK;
    }
    if (rc == RC::PF_INVALIDPAGE) {
      continue;
    }
  }
  return RC::PF_EOF;
}

RC FileHandle::GetPrevPage(PageNum current, PageHandle *page_handle) const {
  for (int i = current - 1; i >= 0; i--) {
    RC rc = GetThisPage(i, page_handle);
    if (rc == RC::OK) {
      return RC::OK;
    }
    if (rc == RC::PF_INVALIDPAGE) {
      continue;
    }
  }

  return RC::PF_EOF;
}

RC FileHandle::GetThisPage(PageNum pageNum, PageHandle *page_handle) const {
  LOG(INFO) << "GetThisPage page_num=" << pageNum;
  BufferPage *buffer_page;
  RC rc = buffer_pool_->GetPage(fd_, pageNum, buffer_page);
  if (rc != RC::OK) {
    return rc;
  }

  // TODO(zhiheng) check header!
  auto *pageHeader = reinterpret_cast<PageHeader *>(buffer_page->Data());
  if (pageHeader->nextFree != USED_PAGE) {
    return RC::PF_INVALIDPAGE;
  }

  *page_handle = PageHandle(pageNum, buffer_page->Data() + PAGE_HEADER_SIZE);

  buffer_page->Pin();

  return RC::OK;
}

RC FileHandle::AllocatePage(PageHandle *page_handle) {
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
    rc = GetThisPage(pageNum, page_handle);
    if (rc != RC::OK) {
      return rc;
    }
    header_.numPages++;
  } else {
    LOG(INFO) << "reuse on free list, firstFree:  " << header_.firstFree;
    // reuse ones on the free list.
    BufferPage *buffer_page;
    PageNum pageNum = header_.firstFree;
    rc = buffer_pool_->GetPage(fd_, pageNum, buffer_page);
    if (rc != RC::OK) {
      return rc;
    }
    PageHeader *pageHeader =
        reinterpret_cast<PageHeader *>(buffer_page->Data());
    // Update the nextFree pointer.
    header_.firstFree = pageHeader->nextFree;
    // Mark this page as used.
    pageHeader->nextFree = USED_PAGE;

    *page_handle = PageHandle(pageNum, buffer_page->Data() + PAGE_HEADER_SIZE);

    buffer_page->MarkDirty();
    buffer_page->Pin();
  }

  is_header_modified = true;

  return RC::OK;
}

RC FileHandle::AllocateNewPage(PageNum pageNum) {
  LOG(INFO) << "AllocateNewPage...";
  assert(IsOpen());

  auto buffer_page = std::unique_ptr<BufferPage>(new BufferPage(fd_, pageNum));
  // Set page header.
  auto *pageHeader = reinterpret_cast<PageHeader *>(buffer_page->Data());
  pageHeader->nextFree = USED_PAGE;
  buffer_page->MarkDirty();
  RC rc = buffer_page->WriteBack();
  if (rc != RC::OK) {
    return rc;
  }
  return buffer_pool_->InsertPage(fd_, pageNum, std::move(buffer_page));
}

RC FileHandle::DisposePage(PageNum pageNum) {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));

  RC rc;
  BufferPage *buffer_page;
  rc = buffer_pool_->GetPage(fd_, pageNum, buffer_page);
  if (rc != RC::OK) {
    return rc;
  }

  if (buffer_page->IsPinned()) {
    return RC::PF_PAGEPINNED;
  }

  // update nextFree.
  PageHeader *pageHeader = reinterpret_cast<PageHeader *>(buffer_page->Data());
  pageHeader->nextFree = header_.firstFree;
  header_.firstFree = pageNum;

  // clear the data section.
  memset(buffer_page->Data() + PAGE_HEADER_SIZE, 0, PAGE_DATA_SIZE);
  buffer_page->MarkDirty();

  is_header_modified = true;
  return RC::OK;
}

RC FileHandle::MarkDirty(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  return buffer_pool_->MarkDirty(fd_, pageNum);
}

RC FileHandle::UnpinPage(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  return buffer_pool_->UnpinPage(fd_, pageNum);
}

RC FileHandle::ForcePages(PageNum pageNum) const {
  assert(IsOpen());
  assert(IsValidPageNum(pageNum));
  if (pageNum == ALL_PAGES) {
    return buffer_pool_->ForceAllPages(fd_);
  }
  return buffer_pool_->ForcePage(fd_, pageNum);
}
} // namespace pf
} // namespace redbase
