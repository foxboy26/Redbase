#pragma once

#include "PF.h"

class PF_Manager
{
  public:
    PF_Manager();
    ~PF_Manager();
    void CreateFile(const char* filename);
    void DestroyFile(const char* filename);
    void OpenFile(const char* filename, PF_FileHandle& fileHandle);
    void CloseFile(PF_FileHanlde& fileHandle);
    void AllocateBlock (char*& buffer);
    void DisposeBlock  (char* buffer);

  private:
    PF_BufferPool* bufferPool;

    RC InitFileHdr(int fd);
    RC ReadFileHdr(PF_FileHandle& fileHandle);
};
