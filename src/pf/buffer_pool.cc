#include "buffer_pool.h"

#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"
#include "src/pf/pf.h"

namespace redbase {
namespace pf {
BufferPage::BufferPage(int fd, PageNum pageNum)
    : fd_(fd), pageNum_(pageNum), isDirty_(false), pinCount_(0) {
  pData_ = new char[PAGE_SIZE];
  memset(pData_, 0, PAGE_SIZE);
}

BufferPage::~BufferPage() {
  delete[] pData_;
  pData_ = nullptr;
}

RC BufferPage::WriteBack() {
  if (!isDirty_) {
    LOG(WARNING) << "page is clean; noop.";
    return RC::OK;
  }

  if (IsPinned()) {
    LOG(WARNING) << "page is still in use, cannot write back.";
    return RC::PF_PAGEPINNED;
  }

  if (lseek(fd_, PageOffset(pageNum_), SEEK_SET) < 0) {
    return RC::PF_UNIX;
  }
  ssize_t numBytes = write(fd_, pData_, PAGE_SIZE);
  if (numBytes < 0) {
    return RC::PF_UNIX;
  }
  if (numBytes < PAGE_SIZE) {
    return RC::PF_INCOMPLETEWRITE;
  }

  return RC::OK;
}

RC BufferPage::Read() {
  LOG(INFO) << "Read page from fd: " << fd_ << ", page_num: " << pageNum_;
  if (lseek(fd_, PageOffset(pageNum_), SEEK_SET) < 0) {
    return RC::PF_UNIX;
  }

  ssize_t numBytes = read(fd_, pData_, PAGE_SIZE);
  if (numBytes < 0) {
    return RC::PF_UNIX;
  }
  if (numBytes < PAGE_SIZE) {
    return RC::PF_INCOMPLETEREAD;
  }

  return RC::OK;
}

///////////////////////////////////////////////////////////////////////////////
// BufferPool

namespace {
bool EvictFunction(const BufferPageKey &key, BufferPage *page) {
  LOG(INFO) << "Evicting: " << key.first << " " << key.second;
  if (page->IsPinned()) {
    // return RC::PAGEPINNED;
    LOG(ERROR) << "Page is pinnned";
    return false;
  }

  if (page->IsDirty()) {
    LOG(INFO) << "PAGE is dirty need to write back to disk.";
    RC rc = page->WriteBack();
    if (rc != RC::OK) {
      LOG(ERROR) << "WriteBack failed with rc: " << rc;
      return false;
      // return rc;
    }
  }

  return true;
}
} // namespace

BufferPool::BufferPool(int bufferSize)
    : bufferSize_(bufferSize), curSize_(0), pool_(bufferSize) {
  pool_.SetEvictFunction(EvictFunction);
}

RC BufferPool::InsertPage(int fd, PageNum pageNum,
                          std::unique_ptr<BufferPage> page) {
  auto key = std::pair<int, PageNum>(fd, pageNum);
  LOG(INFO) << "InsertPage... fd: " << key.first << "page_num: " << key.second;
  if (!pool_.Put(key, std::move(page))) {
    LOG(ERROR) << "failed to insert page: fd=" << key.first
               << ",pageNum=" << key.second;
    return RC::PF_HASHPAGEEXIST;
  }

  return RC::OK;
}

RC BufferPool::GetPage(int fd, PageNum pageNum, BufferPage *&page) {
  BufferPage *res = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (res != nullptr) {
    page = res;
    return RC::OK;
  }

  // Page fault.
  std::unique_ptr<BufferPage> newPage(new BufferPage(fd, pageNum));
  RC rc = newPage->Read();
  if (rc != RC::OK) {
    LOG(ERROR) << "Failed to read new page from file";
    return rc;
  }
  page = newPage.get();

  return InsertPage(fd, pageNum, std::move(newPage));
}

RC BufferPool::MarkDirty(int fd, PageNum pageNum) {
  auto got = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (got == nullptr) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  got->MarkDirty();
  return RC::OK;
}

RC BufferPool::UnpinPage(int fd, PageNum pageNum) {
  auto got = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (got == nullptr) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  got->Unpin();
  return RC::OK;
}

RC BufferPool::ForcePage(int fd, PageNum pageNum) {
  auto got = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (got == nullptr) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  return got->WriteBack();
}

RC BufferPool::ForceAllPages(int fd) {
  for (auto it = pool_.GetIterator(); it.HasNext(); it.Next()) {
    const auto &key = it.Key();
    if (key.first == fd) {
      RC rc = BufferPool::ForcePage(key.first, key.second);
      if (rc != RC::OK) {
        LOG(ERROR) << "ForcePage: failed; abort ForceAllPages";
        return rc;
      }
    }
  }
  return RC::OK;
}

RC BufferPool::AllocateBlock(char *&buffer) { return RC::NOT_IMPLEMENTED; }

RC BufferPool::DisposeBlock(char *buffer) { return RC::NOT_IMPLEMENTED; }
} // namespace pf
} // namespace redbase
