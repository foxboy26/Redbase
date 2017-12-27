#ifndef PF_PF_BUFFERPOOL_H
#define PF_PF_BUFFERPOOL_H

#include <list>
#include <unordered_map>
#include <utility>

namespace std {
template <> struct hash<pair<int, PageNum>> {
public:
  size_t operator()(const pair<int, PageNum> &key) const {
    return static_cast<size_t>(key.first ^ key.second);
  }
};
}; // namespace std

class PF_BufferPool {
public:
  PF_BufferPool(int bufferSize);
  ~PF_BufferPool();

  char *GetPage(int fd, PageNum pageNum);
  void MarkDirty(int fd, PageNum pageNum);
  void UnpinPage(int fd, PageNum pageNum);
  void ForcePages(int fd, PageNum pageNum = ALL_PAGES);
  char *AllocatePage(int fd, PageNum pageNum);
  void AllocateBlock(char *&buffer);
  void DisposeBlock(char *buffer);

private:
  class PF_BufferPage {
  public:
    PF_BufferPage();
    ~PF_BufferPage();
    void Clear();

    int fd;
    PageNum pageNum;
    bool isDirty;
    short pinCount;
    short refBit;
    char *pData;

    friend std::ostream &operator<<(std::ostream &out,
                                    const PF_BufferPage &page);
  };

  int bufferSize;
  size_t pageSize;
  PF_BufferPage *bufferPool;
  std::list<int>::iterator hand;
  std::unordered_map<std::pair<int, PageNum>, int> indexMap;
  std::list<int> freeSlots;
  std::list<int> usedSlots;

  void ForceSinglePage(PF_BufferPage &page);
  void FlushPage(int fd);
  void WritePage(PF_BufferPage &page);
  void ReadPage(PF_BufferPage &page);
  int InternalAllocate(int fd, PageNum pageNum);
  int ChooseNextSlot();

  friend std::ostream &operator<<(std::ostream &out, const PF_BufferPool &pool);
};

#endif // PF_PF_BUFFERPOOL_H
