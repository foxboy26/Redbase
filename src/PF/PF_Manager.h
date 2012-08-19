#pragma once

#include "PF.h"

class PF_Manager
{
public:
     PF_Manager    ();
     ~PF_Manager   ();
  RC CreateFile    (const char* filename);
  RC DestroyFile   (const char* filename);
  RC OpenFile      (const char* filename, PF_FileHandle& fileHandle);
  RC CloseFile     (PF_FileHanlde& fileHandle);
  RC AllocateBlock (char*& buffer);
  RC DisposeBlock  (char* buffer);
};
