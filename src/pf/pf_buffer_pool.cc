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
// LRUCache
//
LRUCache::LRUCache(int size) : bufferSize_(size), curSize_(0) {}
RC LRUCache::Put(const PF_BufferPageKey &key,
                 std::unique_ptr<PF_BufferPage> page) {
  // when buffer is full.
  if (curSize_ == bufferSize_) {
    PF_BufferPageKey victim = lru_queue_.front();
    auto got = pool_.find(victim);
    if (got == pool_.end()) {
      return RC::PF_HASHNOTFOUND;
    }

    // Remove from the pool.
    pool_.erase(got->first);
    lru_queue_.pop();
  }

  // Insert page.
  auto res = pool_.insert(std::make_pair(key, std::move(page)));
  if (!res.second) {
    LOG(ERROR) << "failed to insert page: fd=" << key.first
               << ",pageNum=" << key.second;
    return RC::PF_HASHPAGEEXIST;
  }

  lru_queue_.push(key);

  return RC::OK;
}

PF_BufferPage *LRUCache::Get(const PF_BufferPageKey &key) {
  auto res = pool_.find(key);
  if (res == pool_.end()) {
    return nullptr;
  }
  return res->second.get();
}

///////////////////////////////////////////////////////////////////////////////
// PF_BufferPool
PF_BufferPool::PF_BufferPool(int bufferSize)
    : bufferSize_(bufferSize), curSize_(0) {}

RC PF_BufferPool::InsertPage(int fd, PageNum pageNum,
                             std::unique_ptr<PF_BufferPage> page) {
  // when buffer is full.
  if (curSize_ == bufferSize_) {
    PF_BufferPageKey victim = lru_queue_.front();
    auto got = pool_.find(victim);
    if (got == pool_.end()) {
      return RC::PF_HASHNOTFOUND;
    }

    if (got->second->IsPinned()) {
      return RC::PF_PAGEPINNED;
    }

    if (got->second->IsDirty()) {
      LOG(INFO) << "PAGE is dirty need to write back to disk.";
      RC rc = got->second->WriteBack();
      if (rc != RC::OK) {
        return rc;
      }
    }

    // Remove from the pool.
    pool_.erase(got->first);
    lru_queue_.pop();
  }

  // Insert page.
  auto key = std::pair<int, PageNum>(fd, pageNum);
  auto res = pool_.insert(std::make_pair(key, std::move(page)));
  if (!res.second) {
    LOG(ERROR) << "failed to insert page: fd=" << key.first
               << ",pageNum=" << key.second;
    return RC::PF_HASHPAGEEXIST;
  }

  lru_queue_.push(key);

  return RC::OK;
}

RC PF_BufferPool::GetPage(int fd, PageNum pageNum, PF_BufferPage *&page) {
  auto res = pool_.find(std::pair<int, PageNum>(fd, pageNum));
  // page fault.
  if (res == pool_.end()) {
    std::unique_ptr<PF_BufferPage> newPage(new PF_BufferPage(fd, pageNum));
    RC rc = newPage->Read();
    if (rc != RC::OK) {
      LOG(ERROR) << "Failed to read new page from file";
      return rc;
    }
    page = newPage.get();

    return InsertPage(fd, pageNum, std::move(newPage));
  }

  page = res->second.get();

  return RC::OK;
}

RC PF_BufferPool::MarkDirty(int fd, PageNum pageNum) {
  auto got = pool_.find(std::pair<int, PageNum>(fd, pageNum));
  if (got == pool_.end()) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  got->second->MarkDirty();
  return RC::OK;
}

RC PF_BufferPool::UnpinPage(int fd, PageNum pageNum) {
  auto got = pool_.find(std::pair<int, PageNum>(fd, pageNum));
  if (got == pool_.end()) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  got->second->Unpin();
  return RC::OK;
}

RC PF_BufferPool::ForcePage(int fd, PageNum pageNum) {
  auto res = pool_.find(std::pair<int, PageNum>(fd, pageNum));
  if (res == pool_.end()) {
    LOG(ERROR) << "Page not found in buffer";
    return RC::PF_PAGENOTINBUF;
  }

  return res->second->WriteBack();
}

RC PF_BufferPool::ForceAllPages(int fd) {
  for (const auto &page : pool_) {
    const auto key = page.first;
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
