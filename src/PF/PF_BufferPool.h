#pragma once

#include "PF.h"
#include "PF_HashTable.h"

const int PF_BUFFER_SIZE   = 40;   // Number of pages in the buffer
const int PF_HASH_TBL_SIZE = 20;   // Size of hash table

struct PF_BufferPage
{
  int     fd;
  PageNum pageNum;
  char*   pData;
  bool    isDirty;
  int     pinCount;
};

class PF_BufferPool
{
public:
  PF_BufferPool    (const int numPages);
  ~PF_BufferPool   ();
  RC GetPage       (int fd, PageNum pageNum, char** ppBuffer);
  RC AllocatePage  (int fd, PageNum pageNum, char** ppBuffer);
  RC MarkDirty     (int fd, PageNum pageNum);
  RC UnpinPage     (int fd, PageNum pageNum);
  RC AllocateBlock (char*& buffer);
  RC DisposeBlock  (char* buffer);

private:
  static const int PF_HASH_TBL_SIZE;

  PF_BufferPage* bufferPool;
  PF_HashTable   hashTable;
  int            numPages;
  int            pageSize;

  RC WritePage   (int fd, PageNum pageNum);
  RC ReadPage    (int fd, PageNum pageNum);
};
