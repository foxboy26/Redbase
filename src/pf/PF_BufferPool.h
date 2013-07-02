#pragma once

#include "PF.h"
#include "PF_HashTable.h"

class PF_BufferPool
{
public:
  PF_BufferPool          (int numPages);
  ~PF_BufferPool         ();
  RC GetPage             (int fd, PageNum pageNum, char** ppBuffer);
  RC AllocatePage        (int fd, PageNum pageNum, char** ppBuffer);
  RC MarkDirty           (int fd, PageNum pageNum);
  RC UnpinPage           (int fd, PageNum pageNum);
  RC ForcePage           (int fd, PageNum pageNum = ALL_PAGES);
  RC AllocateBlock       (char*& buffer);
  RC DisposeBlock        (char* buffer);

  // for debug
  RC Print             ();

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
    short   clock;
    char*   pData;
  };

  static const int       PF_HASH_TBL_SIZE = 20;

  PF_BufferPage*         bufferPool;
  PF_HashTable           hashTable;
  int                    numPages;
  int                    pageSize;

  RC FlushPage           (int fd, PageNum pageNum);
  RC WritePage           (PF_BufferPage& page);
  RC ReadPage            (PF_BufferPage& page);
  //RC ReadPage            (int fd, PageNum pageNum, const char* dest);
  RC InternalAllocate    (int fd, PageNum pageNum, int& slot);
  RC ChooseNextSlot      (int& slot);
};
