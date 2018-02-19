#ifndef PF_BUFFERPOOL_H
#define PF_BUFFERPOOL_H

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <utility>

#include "src/pf/internal.h"
#include "src/pf/lru_cache.h"
#include "src/pf/page_handle.h"
#include "src/pf/pf.h"
#include "src/rc.h"

using BufferPageKey = std::pair<int, PageNum>;

namespace std {
template <> struct hash<BufferPageKey> {
public:
  size_t operator()(const BufferPageKey &key) const {
    return static_cast<size_t>(key.first + key.second);
  }
};
}; // namespace std

namespace redbase {
namespace pf {
// not thread-safe.
class BufferPage {
public:
  explicit BufferPage(int fd, PageNum pageNum);
  ~BufferPage();

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

  PageHandle ToPageHandle() { return PageHandle(fd_, pData_); }

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

class BufferPool {
public:
  explicit BufferPool(int bufferSize);
  ~BufferPool() = default;

  RC InsertPage(int fd, PageNum pageNum, std::unique_ptr<BufferPage> page);
  RC GetPage(int fd, PageNum pageNum, BufferPage *&page);
  RC MarkDirty(int fd, PageNum pageNum);
  RC UnpinPage(int fd, PageNum pageNum);
  RC ForcePage(int fd, PageNum pageNum);
  RC ForceAllPages(int fd);

  RC AllocateBlock(char *&buffer);
  RC DisposeBlock(char *buffer);

private:
  int bufferSize_;
  int curSize_;
  LRUCache<BufferPageKey, BufferPage> pool_;
  friend std::ostream &operator<<(std::ostream &out, const BufferPool &pool);
};
} // namespace pf
} // namespace redbase

#endif // BUFFERPOOL_H
