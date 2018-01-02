#ifndef PF_PF_BUFFERPOOL_H
#define PF_PF_BUFFERPOOL_H

#include <cstddef>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>

#include "src/pf/pf.h"
#include "src/pf/pf_internal.h"
#include "src/pf/pf_page_handle.h"
#include "src/rc.h"

using PF_BufferPageKey = std::pair<int, PageNum>;

namespace std {
template <> struct hash<PF_BufferPageKey> {
public:
  size_t operator()(const PF_BufferPageKey &key) const {
    return static_cast<size_t>(key.first ^ key.second);
  }
};
}; // namespace std

class PF_BufferPage {
public:
  explicit PF_BufferPage(int fd, PageNum pageNum);
  ~PF_BufferPage();

  RC Read();      // read page content from disk.
  RC WriteBack(); // Write back the modified content to file on disk. It's a
  // noop if the page is not dirty.
  bool IsDirty() { return isDirty_; }
  bool IsPinned() { return pinCount_ > 0; }
  void MarkDirty() { isDirty_ = true; }
  void Pin() { pinCount_++; }
  void Unpin() {
    if (pinCount_ > 0) {
      pinCount_--;
    }
  }

  char *Data() { return pData_; }

  PF_PageHandle ToPageHandle() { return PF_PageHandle(fd_, pData_); }

  std::string DebugString() {
    // return "{fd=" + fd_ + ",pageNum=" + pageNum_ + ",isDirty=" + isDirty +
    //       ",pinCount=" + pinCount_ + "}";
    return "hahahaha";
  }

private:
  int fd_; // file descriptor that the page refers to.
  PageNum pageNum_;
  bool isDirty_; // dirty -> page content is out-of-sync with the file on disk.
  int pinCount_; // num of time the page is pinned.
  char *pData_;  // content of the page.
};

class LRUCache {
public:
  explicit LRUCache(int size);
  ~LRUCache() = default;

  RC Put(const PF_BufferPageKey &key, std::unique_ptr<PF_BufferPage> page);
  PF_BufferPage *Get(const PF_BufferPageKey &key);

  bool Empty() { return curSize_ == 0; }
  int Capacity() { return bufferSize_; }

private:
  int bufferSize_;
  int curSize_;
  std::queue<PF_BufferPageKey> lru_queue_;
  std::unordered_map<PF_BufferPageKey, std::unique_ptr<PF_BufferPage>> pool_;
};

class PF_BufferPool {
public:
  explicit PF_BufferPool(int bufferSize);
  ~PF_BufferPool() = default;

  RC InsertPage(int fd, PageNum pageNum, std::unique_ptr<PF_BufferPage> page);
  RC GetPage(int fd, PageNum pageNum, PF_BufferPage *&page);
  RC MarkDirty(int fd, PageNum pageNum);
  RC UnpinPage(int fd, PageNum pageNum);
  RC ForcePage(int fd, PageNum pageNum);
  RC ForceAllPages(int fd);

  RC AllocateBlock(char *&buffer);
  RC DisposeBlock(char *buffer);

private:
  int bufferSize_;
  int curSize_;
  std::queue<PF_BufferPageKey> lru_queue_;
  std::unordered_map<PF_BufferPageKey, std::unique_ptr<PF_BufferPage>> pool_;
  friend std::ostream &operator<<(std::ostream &out, const PF_BufferPool &pool);
};

#endif // PF_PF_BUFFERPOOL_H
