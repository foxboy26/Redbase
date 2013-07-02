#include "PF_HashTable.h"

#include <cstdio>
using namespace std;

PF_HashTable::PF_HashTable(unsigned int _numBuckets)
: numBuckets(_numBuckets)
{
  hashTable = new PF_HashEntry* [numBuckets];

  for (int i = 0; i < numBuckets; i++)
  {
    hashTable[i] = NULL;
  }
}

PF_HashTable::~PF_HashTable()
{
  for (int i = 0; i < numBuckets; i++)
  {
    PF_HashEntry* entry = hashTable[i];
    PF_HashEntry* next = entry;
    while (entry != NULL)
    {
      next = entry->next;
      delete entry;
      entry = next;
    }
  }

  delete [] hashTable;
}

RC PF_HashTable::Find(const int fd, const PageNum pageNum, int& slot)
{
  int key = Hash(fd, pageNum);

  if (key < 0)
  {
    return PF_HASHNOTFOUND;
  }

  for (PF_HashEntry* e = hashTable[key]; e != NULL; e = e->next)
  {
    if (e->fd == fd && e->pageNum == pageNum)
    {
      slot = e->slot;
      return OK;
    }
  }

  return PF_HASHNOTFOUND;
}

RC PF_HashTable::Insert(const int fd, const PageNum pageNum, const int slot)
{
  int key = Hash(fd, pageNum);

  if (key < 0)
  {
    return PF_HASHNOTFOUND;
  }

  PF_HashEntry* entry;
  for (entry = hashTable[key]; entry != NULL; entry = entry->next)
  {
    if (entry->fd == fd && entry->pageNum == pageNum)
    {
      return PF_HASHPAGEEXIST;
    }
  }

  entry = new PF_HashEntry;

  entry->next = hashTable[key];
  entry->prev = NULL;
  entry->fd = fd;
  entry->pageNum = pageNum;
  entry->slot = slot;

  if (hashTable[key] != NULL)
  {
    hashTable[key]->prev = entry;
  }
  hashTable[key] = entry;

  return OK;
}

RC PF_HashTable::Delete(const int fd, const PageNum pageNum)
{
  int key = Hash(fd, pageNum);

  if (key < 0)
  {
    return PF_HASHNOTFOUND;
  }

  PF_HashEntry* entry;
  for (entry = hashTable[key]; entry != NULL; entry = entry->next)
  {
    if (entry->fd == fd && entry->pageNum == pageNum)
    {
      if (entry == hashTable[key])
      {
        hashTable[key] = entry->next;  
      }

      if (entry->next != NULL)
      {
        entry->next->prev = entry->prev;
      }

      if (entry->prev != NULL)
      {
        entry->prev->next = entry->next;
      }

      delete entry;

      return OK;
    }
  }

  return PF_HASHNOTFOUND;
}

void PF_HashTable::Print()
{
  for (int i = 0; i < numBuckets; i++)
  {
    printf("Slot %d: ", i);
    bool writeArrow = false;
    for (PF_HashEntry* e = hashTable[i]; e != NULL; e = e->next)
    {
      if (writeArrow)
      {
        printf("->");
      }
      else
      {
        writeArrow = true;
      }
      printf("(%d %d, %d)", e->fd, e->pageNum, e->slot);
    }
    printf("\n");
  }
}
