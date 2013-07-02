#include "PF_BufferPool"

#include <sys/types.h>
#include <unistd.h>

PF_BufferPool::PF_BfferPage::PF_BufferPage() : isDirty(false), pinCount(0), clock(0)
{
  pData = new char[pageSize];
  memset(pData, 0, PF_PAGE_SIZE);
}

PF_BufferPool::PF_BfferPage::~PF_BufferPage()
{
  delete [] pData;
  pData = NULL;
}

PF_BufferPool::PF_BufferPool(int numPages) : hashTable(PF_HASH_TBL_SIZE), numPages(numPages), pageSize(PF_PAGE_SIZE)
{
  bufferPool = new PF_BufferPage[numPages];
}

PF_BufferPool::~PF_BufferPool()
{
  delete [] bufferPool;
  bufferPool = NULL;
}

RC PF_BufferPool::GetPage(int fd, PageNum pageNum, char** ppBuffer)
{
  RC rc;
  int slot;

  rc = hashTable.Find(fd, pageNum, slot);
  if (rc != OK && RC != PF_HASHNOTFOUND) return rc;

  if (rc == PF_HASHNOTFOUND)
  {
    rc = InternalAllocate(fd, pageNum, slot);
    if (rc != OK) return rc;

    rc = ReadPage(bufferPool[slot]);
    if (rc != OK) return rc;
  }

  PF_BufferPage& page = bufferPool[slot];

  page.pinCount++;

  *ppBuffer = page.pData;

  page.clock = 1;

  return OK;
}

RC PF_BufferPool::AllocatePage(int fd, PageNum pageNum, char** ppBuffer)
{
  RC rc;

  rc = InternalAllocate(fd, pageNum, slot);

  if (rc != OK) return rc;

  PF_BufferPage& page = bufferPool[slot];

  page.pinCount++;

  *ppBuffer = page.pData;

  page.clock = 1;

  return OK;
}

RC PF_BufferPool::MarkDirty(int fd, PageNum pageNum)
{
  RC rc;
  int slot;

  rc = hashTable.Find(fd, pageNum, slot);

  if (rc != OK) return rc;

  PF_BufferPage& page = bufferPool[slot];

  if (page.pinCount == 0) return PF_PAGEUNPINNED;

  page.isDirty = true;
  
  page.clock = 1;
  
  return OK;
}

RC PF_BufferPool::UnpinPage(int fd, PageNum pageNum)
{
  RC rc;
  int slot;

  rc = hashTable.Find(fd, pageNum, slot);

  if (rc != OK) return rc;

  if (bufferPool[slot].pinCount == 0) return PF_PAGEUNPINNED;

  bufferPool[slot].pinCount--;

  page.clock = 0;

  return OK;
}

RC PF_BufferPool::ForcePage(int fd, PageNum pageNum = ALL_PAGES)
{
  RC rc;
  PF_BufferPage& page

  if (pageNum == ALL_PAGES)
  {
    for (int i = 0; i < numPages; i++)
    {
      if (bufferPool[i].fd == fd)
      {
        rc = WritePage(bufferPool[i]);
        if (rc != OK) return rc;
      }
    }
  }
  else
  {
    int slot;

    rc = hashTable.Find(fd, pageNum, slot);
    if (rc != OK) return rc;

    rc = WritePage(bufferPool[slot]);
    if (rc != OK) return rc;
  }

  return OK;
}

RC PF_BufferPool::AllocateBlock(char*& buffer)
{
  return OK;  
}

RC PF_BufferPool::DisposeBlock(char* buffer)
{
  return OK;  
}

RC FlushPage(PF_BufferPage& page)
{
  RC rc;
  int slot;

  rc = hashTable.Find(page.fd, page.pageNum, slot);
  if (rc != OK) return rc;

  rc = WritePage(bufferPool[slot]);
  if (rc != OK) return rc;

  rc = hashTable.Delete(page.fd, page.pageNum);
  if (rc != OK) return rc;

  page.pinCount = 0;

  page.clock = 0;

  return OK;  
}
/*
RC FlushPage(int fd, PageNum pageNum)
{
  RC rc;
  int slot;

  rc = hashTable.Find(fd, pageNum, slot);
  if (rc != OK) return rc;

  rc = WritePage(bufferPool[slot]);
  if (rc != OK) return rc;

  rc = hashTable.Delete(fd, pageNum);
  if (rc != OK) return rc;

  page.pinCount = 0;

  page.clock = 0;

  return OK;  
}*/


RC PF_BufferPool::WritePage(PF_BufferPage& page)
{
  if (page.isDirty)
  {
    off_t offset = page.pageNum * this->pageSize;

    if (lseek(page.fd, offset, SEEK_SET) < 0)
      return PF_UNIX;
    
    size_t numBytes = write(page.fd, page.pData, pageSize);

    if (numBytes < 0)
      return PF_UNIX;

    if (numBytes < pageSize)
      return PF_INCOMPLETEWRITE;

    page.isDirty = false;
  }

  return OK;
}

RC PF_BufferPool::ReadPage(PF_BufferPage& page)
{
  off_t offset = page.pageNum * pageSize;

  if (lseek(page.fd, offset, SEEK_SET) < 0)
    return PF_UNIX;
  
  size_t numBytes = read(page.fd, page.pData, pageSize);

  if (numBytes < 0)
    return PF_UNIX;

  if (numBytes < pageSize)
    return PF_INCOMPLETEREAD;

  return OK;
}

/*
RC PF_BufferPool::WritePage(int fd, PageNum pageNum, const char* src)
{
  off_t offset = pageNum * pageSize;

  if (lseek(fd, offset, SEEK_SET) < 0)
    return PF_UNIX;
  
  int numBytes = write(fd, src, pageSize);

  if (numBytes < 0)
    return PF_UNIX;

  if (numBytes < pageSize)
    return PF_INCOMPLETEWRITE;

  return OK;
}

RC PF_BufferPool::ReadPage(int fd, PageNum pageNum, const char* dest)
{
  off_t offset = pageNum * pageSize;

  if (lseek(fd, offset, SEEK_SET) < 0)
    return PF_UNIX;
  
  int numBytes = read(fd, dest, pageSize);

  if (numBytes < 0)
    return PF_UNIX;

  if (numBytes < pageSize)
    return PF_INCOMPLETEREAD;

  return OK;
}*/

RC PF_BufferPool::InternalAllocate(int fd, PageNum pageNum, int& slot)
{
  RC rc;
  
  if (/*free*/)
  {
    slot = free;
  }
  else
  {
    rc = ChooseNextSlot(slot);
    if (rc != OK) return rc;

    rc = FlushPage(bufferPool[slot]);
    if (rc != OK) return rc;

    //PF_BufferPage& oldPage = bufferPool[slot];
    //rc = FlushPage(oldPage.fd, oldPage.pageNum);
  }

  rc = hashTable.Insert(fd, pageNum, slot);

  if (rc != OK) return rc;
  
  return OK;
}

RC PF_BufferPool::ChooseNextSlot(int& slot)
{
  // using clock algorithm (second chance)
  int chance = 0;

  while (chance < 2)
  {
    hand = (hand == numPages)? 0 : hand + 1;

    if (bufferPool[hand].clock == 0)
      chance++;
    else
      bufferPool[hand].clock == 0;
  }

  slot = hand;

  return OK;
}

RC PF_BufferPool::Print()
{
  return OK;
}
