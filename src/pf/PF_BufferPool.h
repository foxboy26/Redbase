#pragma once

#include "PF.h"

class PF_BufferPool
{
  public:
    PF_BufferPool(int bufferSize);
    ~PF_BufferPool();

    void GetPage(int fd, PageNum pageNum, char** ppBuffer);
    void AllocatePage(int fd, PageNum pageNum, char** ppBuffer);
    void MarkDirty(int fd, PageNum pageNum);
    void UnpinPage(int fd, PageNum pageNum);
    void ForcePage(int fd, PageNum pageNum = ALL_PAGES);
    void AllocateBlock(char*& buffer);
    void DisposeBlock(char* buffer);

  private:
    class PF_BufferPage
    {
      public:
        PF_BufferPage();
        ~PF_BufferPage();
      private:
        int     fd;
        PageNum pageNum;
        bool    isDirty;
        short   pinCount;
        short   refBit;
        char*   pData;
    };

    int bufferSize;
    int pageSize;
    PF_BufferPage* bufferPool;
    int hand;
    std::unordered_map<std::pair<int, PageNum>, int> indexMap;

    RC FlushPage(int fd, PageNum pageNum);
    RC WritePage(PF_BufferPage& page);
    RC ReadPage(PF_BufferPage& page);
    int InternalAllocate(int fd, PageNum pageNum);
    int ChooseNextSlot();
};
