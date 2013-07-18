#include "PF_BufferPool.h"

#include <sys/types.h>
#include <unistd.h>

#include <unordered_map>

PF_BufferPool::PF_BufferPage::PF_BufferPage() : isDirty(false), pinCount(0), refBit(0)
{
  pData = new char[pageSize];
  memset(pData, 0, PF_PAGE_SIZE);
}

PF_BufferPool::PF_BufferPage::~PF_BufferPage()
{
  delete [] pData;
  pData = nullptr;
}

PF_BufferPool::PF_BufferPool(int bufferSize) : bufferSize(bufferSize), pageSize(PF_PAGE_SIZE), hand(-1)
{
  bufferPool = new PF_BufferPage[bufferSize];
}

PF_BufferPool::~PF_BufferPool()
{
  delete [] bufferPool;
  bufferPool = nullptr;
}

void PF_BufferPool::GetPage(int fd, PageNum pageNum, char*& pData)
{
  int slot;

  auto res = indexMap.find(std::make_pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
  {
    // page fault
    slot = InternalAllocate(fd, pageNum);

    bufferPool[slot].fd = fd;
    bufferPool[slot].pageNum = pageNum;

    ReadPage(bufferPool[slot]);
  }
  else
  {
    // hit
    slot = res->second;
  }

  bufferPool[slot].pinCount++;

  pData = page.pData;
}

void PF_BufferPool::MarkDirty(int fd, PageNum pageNum)
{
  auto res = indexMap.find(std::make_pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
    throw PF_Exception(PF_Exception::PAGENOTINBUF);

  int slot = res->second;

  PF_BufferPage& page = bufferPool[slot];

  if (page.pinCount == 0)
    throw PF_Exception(PF_Exception::PAGEUNPINNED);

  page.isDirty = true;
}

void PF_BufferPool::UnpinPage(int fd, PageNum pageNum)
{
  auto res = indexMap.get(std::make_pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
    throw PF_Exception(PF_Exception::PAGENOTINBUF);

  int slot = res->second;

  if (bufferPool[slot].pinCount == 0)
    throw PF_Exception(PF_Exception::PAGEUNPINNED);

  bufferPool[slot].pinCount--;
}

void PF_BufferPool::ForcePage(int fd, PageNum pageNum = ALL_PAGES)
{
  RC rc;
  PF_BufferPage& page

  if (pageNum == ALL_PAGES)
  {
    for (int i = 0; i < bufferSize; i++)
    {
      if (bufferPool[i].fd == fd)
      {
        FlushPage(bufferPool[i]);
      }
    }
  }
  else
  {
    auto res = indexMap.get(std::make_pair<int, PageNum>(fd, pageNum));
    if (res == indexMap.end())
      throw PF_Exception(PF_Exception::PAGENOTINBUF);

    FlushPage(bufferPool[res->second]);
  }
}

void PF_BufferPool::AllocateBlock(char*& buffer)
{
  throw PF_Exception(PF_Exception::NOTIMPLEMENTED);
}

void PF_BufferPool::DisposeBlock(char* buffer)
{
  throw PF_Exception(PF_Exception::NOTIMPLEMENTED);
}

void FlushPage(PF_BufferPage& page)
{
  std::pair<int, PageNum> key = std::make_pair<int, PageNum> (page.fd, page.pageNum);

  auto res = indexMap.find(key);
  if (res != indexMap.end())
  {
    if (page.pinCount > 0)
      throw PF_Exception(PF_Exception::PAGEPINNED);

    WritePage(page);

    indexMap.remove(key);

    page.pinCount = 0;
    page.ref = 0;
    page.isDirty = false;
  }
  else
    throw PF_Exception(PF_Exception::PAGENOTINBUF);
}

void PF_BufferPool::WritePage(PF_BufferPage& page)
{
  if (page.isDirty)
  {
    off_t offset = page.pageNum * this->pageSize;

    if (lseek(page.fd, offset, SEEK_SET) < 0)
      throw PF_Exception(PF_Exception::UNIX);
    
    size_t numBytes = write(page.fd, page.pData, pageSize);

    if (numBytes < 0)
      throw PF_Exception(PF_Exception::UNIX);

    if (numBytes < pageSize)
      throw PF_Exception(PF_Exception::INCOMPLETEWRITE);

    page.isDirty = false;
  }

  return OK;
}

void PF_BufferPool::ReadPage(PF_BufferPage& page)
{
  off_t offset = page.pageNum * this->pageSize;

  if (lseek(page.fd, offset, SEEK_SET) < 0)
    throw PF_Exception(PF_Exception::UNIX);
  
  size_t numBytes = read(page.fd, page.pData, pageSize);

  if (numBytes < 0)
    throw PF_Exception(PF_Exception::UNIX);

  if (numBytes < pageSize)
    throw PF_Exception(PF_Exception::INCOMPLETEREAD);
}

/*
 * Choose the next free slot, flush page if the slot is exists
 * Update indexMap
 * Return slot
 */
int PF_BufferPool::InternalAllocate(int fd, PageNum pageNum)
{
  int slot = ChooseNextSlot();

  if (!bufferPool[slot].isFree())
  {
    FlushPage(bufferPool[slot]);
  }

  indexMap.put(std::make_pair<int, PageNum>(fd, pageNum));

  return slot;
}

// using clock algorithm (second chance)
int PF_BufferPool::ChooseNextSlot()
{
  int slot = -1;

  while (true)
  {
    hand++;
    if (hand == bufferSize)
      hand = 0;

    if (bufferPool[hand].refBit == 0) {
      bufferPool[hand].refBit = 1;
      slot = hand;
      break;
    }
    else {
      bufferPool[hand].refBit == 0;
    }
  }

  return slot;
}
