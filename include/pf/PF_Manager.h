#ifndef PF_PF_MANAGER_H
#define PF_PF_MANAGER_H

#include "PF.h"
#include "PF_Internal.h"
#include "PF_BufferPool.h"

class PF_Manager
{
public:
  PF_Manager();
  ~PF_Manager();
  void CreateFile(const char* filename);
  void DestroyFile(const char* filename);
  void OpenFile(const char* filename, PF_FileHandle& fileHandle);
  void CloseFile(PF_FileHandle& fileHandle);
  void AllocateBlock (char*& buffer);
  void DisposeBlock  (char* buffer);

private:
  PF_BufferPool* bufferPool;

  void InitFileHdr(int fd);
  void ReadFileHdr(PF_FileHandle& fileHandle);
};

#endif // PF_PF_MANAGER;
