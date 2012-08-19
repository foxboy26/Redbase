#include "PF.h"

#include <stdio.h>

PF_Manager::PF_Manager()
{

}

PF_Manager::~PF_Manager()
{

}

RC PF_Manager::CreateFile(const char* filename)
{
  
  return OK;
}

RC PF_Manager::DestroyFile(const char* filename);
{

}

RC PF_Manager::OpenFile(const char* filename, PF_FileHandle& fileHandle);
{

}

RC PF_Manager::CloseFile(PF_FileHanlde& fileHandle);
{

}

RC PF_Manager::AllocateBlock(char*& buffer);
{

}

RC PF_Manager::DisposeBlock(char* buffer);
{

}
