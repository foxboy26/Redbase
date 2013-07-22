#include "PF_BufferPool.h"

#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

PF_BufferPool::PF_BufferPage::PF_BufferPage() : isDirty(false), pinCount(0), refBit(0)
{
  pData = new char[PF_PAGE_SIZE];
  memset(pData, 0, PF_PAGE_SIZE);
}

PF_BufferPool::PF_BufferPage::~PF_BufferPage()
{
  delete [] pData;
  pData = nullptr;
}

void PF_BufferPool::PF_BufferPage::Clear()
{
  isDirty = false;
  pinCount = 0;
  refBit = 0;
  memset(pData, 0, PF_PAGE_SIZE);
}

PF_BufferPool::PF_BufferPool(int _bufferSize) : bufferSize(_bufferSize), pageSize(PF_PAGE_SIZE)
{
  bufferPool = new PF_BufferPage[bufferSize];
  for (int i = 0; i < bufferSize; i++)
  {
    freeSlots.push_back(i);
  }
  hand = usedSlots.begin();
}

PF_BufferPool::~PF_BufferPool()
{
  delete [] bufferPool;
  bufferPool = nullptr;
}

void PF_BufferPool::GetPage(int fd, PageNum pageNum, char*& pData)
{
  int slot;

  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
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

  pData = bufferPool[slot].pData;
}

void PF_BufferPool::MarkDirty(int fd, PageNum pageNum)
{
  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
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
  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
    throw PF_Exception(PF_Exception::PAGENOTINBUF);

  int slot = res->second;

  if (bufferPool[slot].pinCount == 0)
    throw PF_Exception(PF_Exception::PAGEUNPINNED);

  bufferPool[slot].pinCount--;
}

void PF_BufferPool::ForcePages(int fd, PageNum pageNum)
{
  if (pageNum == ALL_PAGES)
  {
    for (int i = 0; i < bufferSize; i++)
    {
      if (bufferPool[i].fd == fd)
      {
        ForceSinglePage(bufferPool[i]);
      }
    }
  }
  else
  {
    auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
    if (res == indexMap.end())
      throw PF_Exception(PF_Exception::PAGENOTINBUF);

    ForceSinglePage(bufferPool[res->second]);
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

void PF_BufferPool::ForceSinglePage(PF_BufferPage& page)
{
  std::pair<int, PageNum> key = std::pair<int, PageNum> (page.fd, page.pageNum);

  auto res = indexMap.find(key);
  if (res != indexMap.end())
  {
    if (page.pinCount > 0)
    {
      std::cerr << "warning page pinnned" << std::endl;
    }
    
    if (page.isDirty)
    {
      WritePage(page);

      page.isDirty = false;
    }
  }
  else
    throw PF_Exception(PF_Exception::PAGENOTINBUF);
}

/*
 * Require: page in the buffer pool.
 */
void PF_BufferPool::FlushPage(int fd)
{
  for (int i = 0; i < bufferSize; i++)
  {
    if (bufferPool[i].fd == fd)
    {
      ForceSinglePage(bufferPool[i]);
      indexMap.erase(std::pair<int, PageNum>(fd, bufferPool[i].pageNum));
      bufferPool[i].Clear();
      usedSlots.remove(i);
      freeSlots.push_back(i);
    }
  }
}

void PF_BufferPool::WritePage(PF_BufferPage& page)
{
  if (page.isDirty)
  {
    off_t offset = page.pageNum * this->pageSize;

    if (lseek(page.fd, offset, SEEK_SET) < 0)
      throw PF_Exception(PF_Exception::UNIX);
    
    ssize_t numBytes = write(page.fd, page.pData, pageSize);

    if (numBytes < 0)
      throw PF_Exception(PF_Exception::UNIX);

    if (numBytes < pageSize)
      throw PF_Exception(PF_Exception::INCOMPLETEWRITE);

    page.isDirty = false;
  }
}

void PF_BufferPool::ReadPage(PF_BufferPage& page)
{
  off_t offset = page.pageNum * this->pageSize;

  if (lseek(page.fd, offset, SEEK_SET) < 0)
    throw PF_Exception(PF_Exception::UNIX);
  
  ssize_t numBytes = read(page.fd, page.pData, pageSize);

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
  int slot;
  if (!freeSlots.empty())
  {
    slot = freeSlots.front();
    freeSlots.pop_front();
  }
  else
  {
    slot = ChooseNextSlot();

    ForceSinglePage(bufferPool[slot]);

    auto key = std::pair<int, PageNum> (bufferPool[slot].fd, bufferPool[slot].pageNum);
    indexMap.erase(key);

    bufferPool[slot].Clear();
  }

  indexMap.insert({std::pair<int, PageNum>(fd, pageNum), slot});

  usedSlots.push_back(slot);

  return slot;
}

// using clock algorithm (second chance)
int PF_BufferPool::ChooseNextSlot()
{
  int slot = -1;

  while (true)
  {
    hand++;
    if (hand == usedSlots.end())
      hand = usedSlots.begin();

    if (bufferPool[*hand].refBit == 0)
    {
      bufferPool[*hand].refBit = 1;
      slot = *hand;
      break;
    }
    else
    {
      bufferPool[*hand].refBit = 0;
    }
  }

  return slot;
}
