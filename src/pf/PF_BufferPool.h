#pragma once

#include "PF.h"

#include <unordered_map>
#include <utility>
#include <list>

namespace std {
  template <>
    struct hash<pair<int, PageNum>> {
      public :
        size_t operator()(const pair<int, PageNum> &key) const
        {
          return static_cast<size_t>(key.first ^ key.second);
        }
    };
};

class PF_BufferPool
{
  public:
    PF_BufferPool(int bufferSize);
    ~PF_BufferPool();

    void GetPage(int fd, PageNum pageNum, char*& ppBuffer);
    void MarkDirty(int fd, PageNum pageNum);
    void UnpinPage(int fd, PageNum pageNum);
    void ForcePages(int fd, PageNum pageNum = ALL_PAGES);
    void AllocateBlock(char*& buffer);
    void DisposeBlock(char* buffer);

  private:
    class PF_BufferPage
    {
      public:
        PF_BufferPage();
        ~PF_BufferPage();
        void Clear();

        int     fd;
        PageNum pageNum;
        bool    isDirty;
        short   pinCount;
        short   refBit;
        char*   pData;
    };

    int bufferSize;
    size_t pageSize;
    PF_BufferPage* bufferPool;
    std::list<int>::iterator hand;
    std::unordered_map<std::pair<int, PageNum>, int> indexMap;
    std::list<int> freeSlots;
    std::list<int> usedSlots;

    void ForceSinglePage(PF_BufferPage& page);
    void FlushPage(int fd);
    void WritePage(PF_BufferPage& page);
    void ReadPage(PF_BufferPage& page);
    int InternalAllocate(int fd, PageNum pageNum);
    int ChooseNextSlot();
};
