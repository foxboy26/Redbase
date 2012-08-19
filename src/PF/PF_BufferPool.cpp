#include "PF_BufferPool"

PF_BufferPool::PF_HASH_TBL_SIZE = 20;

PF_BufferPool::PF_BufferPool(const int numPages)
:hashTable(PF_HASH_TBL_SIZE), numPages(numPages), pageSize(PF_PAGE_SIZE)
{
  bufferPool = new PF_BufferPage[numPages];

  for (int i = 0; i < numPages; i++)
  {
    bufferPool[i].pData = new char[pageSize];
    
    memset(bufferPool[i].pData, 0, pageSize);
  }
}

PF_BufferPool::~PF_BufferPool()
{
  for (int i = 0; i < numPages; i++)
  {
    delete [] bufferPool[i].pData;
  }
  
  delete [] bufferPool;
}

RC PF_BufferPool::GetPage(int fd, PageNum pageNum, char** ppBuffer)
{
  RC rc;
  int slot;

  rc = hashTable.Find(fd, pageNum, slot);
  if (rc == PF_HASHNOTFOUND)
  {
    bufferPool[slot].pinCount++;
  }
  else
  {
    
  }

  *ppBuffer = bufferPool[slot].pData;

  return OK;
}
