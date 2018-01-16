#include "pf_buffer_pool.h"

#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"
#include "src/pf/pf.h"

PF_BufferPage::PF_BufferPage(int fd, PageNum pageNum)
    : fd_(fd), pageNum_(pageNum), isDirty_(false), pinCount_(0) {
  pData_ = new char[PF_PAGE_SIZE];
  memset(pData_, 0, PF_PAGE_SIZE);
}

PF_BufferPage::~PF_BufferPage() {
  delete[] pData_;
  pData_ = nullptr;
}

RC PF_BufferPage::WriteBack() {
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
  ssize_t numBytes = write(fd_, pData_, PF_PAGE_SIZE);
  if (numBytes < 0) {
    return RC::PF_UNIX;
  }
  if (numBytes < PF_PAGE_SIZE) {
    return RC::PF_INCOMPLETEWRITE;
  }

  return RC::OK;
}

RC PF_BufferPage::Read() {
  LOG(INFO) << "Read page from " << fd_ << ", " << pageNum_;
  if (lseek(fd_, PageOffset(pageNum_), SEEK_SET) < 0) {
    return RC::PF_UNIX;
  }

  ssize_t numBytes = read(fd_, pData_, PF_PAGE_SIZE);
  if (numBytes < 0) {
    return RC::PF_UNIX;
  }
  if (numBytes < PF_PAGE_SIZE) {
    return RC::PF_INCOMPLETEREAD;
  }

  return RC::OK;
}

///////////////////////////////////////////////////////////////////////////////
// PF_BufferPool

namespace {
bool EvictFunction(const PF_BufferPageKey &key, PF_BufferPage *page) {
  LOG(INFO) << "Evicting: " << key.first << " " << key.second;
  if (page->IsPinned()) {
    // return RC::PF_PAGEPINNED;
    return false;
  }

  if (page->IsDirty()) {
    LOG(INFO) << "PAGE is dirty need to write back to disk.";
    RC rc = page->WriteBack();
    if (rc != RC::OK) {
      return false;
      // return rc;
    }
  }

  return true;
}
} // namespace

PF_BufferPool::PF_BufferPool(int bufferSize)
    : bufferSize_(bufferSize), curSize_(0), pool_(bufferSize) {
  pool_.SetEvictFunction(EvictFunction);
}

RC PF_BufferPool::InsertPage(int fd, PageNum pageNum,
                             std::unique_ptr<PF_BufferPage> page) {
  auto key = std::pair<int, PageNum>(fd, pageNum);
  if (!pool_.Put(key, std::move(page))) {
    LOG(ERROR) << "failed to insert page: fd=" << key.first
               << ",pageNum=" << key.second;
    return RC::PF_HASHPAGEEXIST;
  }

  return RC::OK;
}

RC PF_BufferPool::GetPage(int fd, PageNum pageNum, PF_BufferPage *&page) {
  PF_BufferPage *res = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (res != nullptr) {
    page = res;
    return RC::OK;
  }

  // Page fault.
  std::unique_ptr<PF_BufferPage> newPage(new PF_BufferPage(fd, pageNum));
  RC rc = newPage->Read();
  if (rc != RC::OK) {
    LOG(ERROR) << "Failed to read new page from file";
    return rc;
  }
  page = newPage.get();

  return InsertPage(fd, pageNum, std::move(newPage));
}

RC PF_BufferPool::MarkDirty(int fd, PageNum pageNum) {
  auto got = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (got == nullptr) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  got->MarkDirty();
  return RC::OK;
}

RC PF_BufferPool::UnpinPage(int fd, PageNum pageNum) {
  auto got = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (got == nullptr) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  got->Unpin();
  return RC::OK;
}

RC PF_BufferPool::ForcePage(int fd, PageNum pageNum) {
  auto got = pool_.Get(std::pair<int, PageNum>(fd, pageNum));
  if (got == nullptr) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  return got->WriteBack();
}

RC PF_BufferPool::ForceAllPages(int fd) {
  for (auto it = pool_.GetIterator(); it.HasNext(); it.Next()) {
    const auto &key = it.Key();
    if (key.first == fd) {
      RC rc = PF_BufferPool::ForcePage(key.first, key.second);
      if (rc != RC::OK) {
        LOG(ERROR) << "ForcePage: failed; abort ForceAllPages";
        return rc;
      }
    }
  }
  return RC::OK;
}

RC PF_BufferPool::AllocateBlock(char *&buffer) { return RC::NOT_IMPLEMENTED; }

RC PF_BufferPool::DisposeBlock(char *buffer) { return RC::NOT_IMPLEMENTED; }

// std::ostream &operator<<(std::ostream &out, const PF_BufferPool &pool) {
//   out << "bufferSize: " << pool.bufferSize << ", ";
//   out << "pageSize: " << pool.pageSize << ", ";
//   out << "hand: " << *pool.hand << ", ";
//
//   out << "freeSlots: ";
//   out << "[";
//   bool isFirst = true;
//   for (auto it = pool.freeSlots.begin(); it != pool.freeSlots.end(); ++it)
//   {
//     if (isFirst) {
//       out << *it;
//       isFirst = false;
//     } else {
//       out << ", " << *it;
//     }
//   }
//   out << "], ";
//
//   out << "usedSlots: ";
//   out << "[";
//   isFirst = true;
//   for (auto it = pool.usedSlots.begin(); it != pool.usedSlots.end(); ++it)
//   {
//     if (isFirst) {
//       out << *it;
//       isFirst = false;
//     } else {
//       out << ", " << *it;
//     }
//   }
//   out << "]" << std::endl;
//
//   for (auto it = pool.usedSlots.begin(); it != pool.usedSlots.end(); ++it)
//   {
//     out << "Page " << *it << " " << pool.bufferPool[*it] << std::endl;
//   }
//
//   return out;
// }
//
/*std::ostream &operator<<(std::ostream &out,
                         const PF_BufferPool::PF_BufferPage &page) {
  out << "fd: " << page.fd << ", ";
  out << "pageNum: " << page.pageNum << ", ";
  out << "isDirty: " << page.isDirty << ", ";
  out << "pinCount: " << page.pinCount << ", ";
  out << "refBit: " << page.refBit << ", ";
  out << "pData: ";
  for (int i = 0; i < 16; i++)
    out << page.pData[i];

  return out;
}*/
