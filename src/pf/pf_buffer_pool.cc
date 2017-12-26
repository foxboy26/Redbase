#include "PF_BufferPool.h"

#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

std::ostream& operator<< (std::ostream& out, const PF_BufferPool::PF_BufferPage& page)
{
  out << "fd: " << page.fd << ", ";
  out << "pageNum: " << page.pageNum << ", ";
  out << "isDirty: " << page.isDirty << ", ";
  out << "pinCount: " << page.pinCount << ", ";
  out << "refBit: " << page.refBit << ", ";
  out << "pData: ";
  for (int i = 0; i < 16; i++)
    out << page.pData[i];

  return out;
}

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

char* PF_BufferPool::GetPage(int fd, PageNum pageNum)
{
  int slot;

  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
  {
    // page fault
    std::cout << "page fault" << std::endl;

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
  bufferPool[slot].refBit = 1;

  return bufferPool[slot].pData;
}

void PF_BufferPool::MarkDirty(int fd, PageNum pageNum)
{
  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
    throw PF_Exception(PF_Exception::PAGE_NOT_IN_BUF);

  int slot = res->second;

  PF_BufferPage& page = bufferPool[slot];

  if (page.pinCount == 0)
    throw PF_Exception(PF_Exception::PAGE_UNPINNED);

  page.isDirty = true;
}

void PF_BufferPool::UnpinPage(int fd, PageNum pageNum)
{
  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
  if (res == indexMap.end())
    throw PF_Exception(PF_Exception::PAGE_NOT_IN_BUF);

  int slot = res->second;

  if (bufferPool[slot].pinCount == 0)
    throw PF_Exception(PF_Exception::PAGE_UNPINNED);

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
      throw PF_Exception(PF_Exception::PAGE_NOT_IN_BUF);

    ForceSinglePage(bufferPool[res->second]);
  }
}

char* PF_BufferPool::AllocatePage(int fd, PageNum pageNum) {

  auto res = indexMap.find(std::pair<int, PageNum>(fd, pageNum));
  if (res != indexMap.end()) {
    throw PF_Exception(PF_Exception::PAGE_IN_BUF);
  }

  int slot = InternalAllocate(fd, pageNum);

  bufferPool[slot].fd = fd;
  bufferPool[slot].pageNum = pageNum;

  bufferPool[slot].pinCount++;
  bufferPool[slot].refBit = 1;

  return bufferPool[slot].pData;
}

void PF_BufferPool::AllocateBlock(char*& buffer)
{
  throw PF_Exception(PF_Exception::NOT_IMPLEMENTED);
}

void PF_BufferPool::DisposeBlock(char* buffer)
{
  throw PF_Exception(PF_Exception::NOT_IMPLEMENTED);
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
    throw PF_Exception(PF_Exception::PAGE_NOT_IN_BUF);
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
      throw PF_Exception(PF_Exception::UNIX_ERROR);

    ssize_t numBytes = write(page.fd, page.pData, pageSize);

    if (numBytes < 0)
      throw PF_Exception(PF_Exception::UNIX_ERROR);

    if (numBytes < pageSize)
      throw PF_Exception(PF_Exception::INCOMPLETE_WRITE);

    page.isDirty = false;
  }
}

void PF_BufferPool::ReadPage(PF_BufferPage& page)
{
  std::cout << "readpage from " << page.fd << ", " << page.pageNum << std::endl;

  off_t offset = page.pageNum * this->pageSize;

  if (lseek(page.fd, offset, SEEK_SET) < 0)
    throw PF_Exception(PF_Exception::UNIX_ERROR);

  ssize_t numBytes = read(page.fd, page.pData, pageSize);

  if (numBytes < 0)
    throw PF_Exception(PF_Exception::UNIX_ERROR);

  if (numBytes < pageSize)
    throw PF_Exception(PF_Exception::IN_COMPLETEREAD);
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
    usedSlots.push_back(slot);
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

std::ostream& operator<< (std::ostream& out, const PF_BufferPool& pool)
{
  out << "bufferSize: " << pool.bufferSize << ", ";
  out << "pageSize: " << pool.pageSize << ", ";
  out << "hand: " << *pool.hand << ", ";

  out << "freeSlots: ";
  out << "[";
  bool isFirst = true;
  for (auto it = pool.freeSlots.begin(); it != pool.freeSlots.end(); ++it)
  {
    if (isFirst)
    {
      out << *it;
      isFirst = false;
    }
    else
    {
      out << ", " << *it;
    }
  }
  out << "], ";

  out << "usedSlots: ";
  out << "[";
  isFirst = true;
  for (auto it = pool.usedSlots.begin(); it != pool.usedSlots.end(); ++it)
  {
    if (isFirst)
    {
      out << *it;
      isFirst = false;
    }
    else
    {
      out << ", " << *it;
    }
  }
  out << "]" << std::endl;

  for (auto it = pool.usedSlots.begin(); it != pool.usedSlots.end(); ++it)
  {
    out << "Page " << *it << " " << pool.bufferPool[*it] << std::endl;
  }

  return out;
}
