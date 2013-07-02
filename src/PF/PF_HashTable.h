#pragma once

#include "PF.h"

struct PF_HashEntry
{
  PF_HashEntry* next;
  PF_HashEntry* prev;
  int           fd;
  PageNum       pageNum;
  int           slot;
};

class PF_HashTable
{
public:
  PF_HashTable  (int numBuckets);
  ~PF_HashTable ();

  RC Find       (const int fd, const PageNum pageNum, int& slot);
  RC Insert     (const int fd, const PageNum pageNum, const int slot);
  RC Delete     (const int fd, const PageNum pageNum);
  // for debug
  void Print    ();
private:
  int numBuckets;

  PF_HashEntry** hashTable;

  int Hash(const int fd, const PageNum pageNum) const
  {
    return (fd + pageNum) % numBuckets;
  }
};
