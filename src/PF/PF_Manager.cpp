#include "PF_Manager.h"

#include <stdio.h>
#include <unistd.h>

PF_Manager::PF_Manager()
{
  bufferPool = new BufferPool(PF_BUFFER_SIZE);
}

PF_Manager::~PF_Manager()
{
  delete bufferPool;
}

RC PF_Manager::CreateFile(const char* filename)
{
  RC rc;
  int fd;

  fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, 0600);
  if (fd == -1) return PF_UNIX;

  rc = InitFileHdr(fd);
  if (rc != OK)
  {
    if (close(fd) == -1) return PF_UNIX;

    if (remove(filename) == -1) return PF_UNIX;

    return rc;
  }

  if (close(fd) == -1) return PF_UNIX;

  return OK;
}

RC PF_Manager::DestroyFile(const char* filename);
{
  if (remove(filename) == -1) return PF_UNIX;

  return OK;
}

RC PF_Manager::OpenFile(const char* filename, PF_FileHandle& fileHandle);
{
  int RC;
  
  if (fileHandle.isOpen) return PF_FILEOPEN;

  fileHandle.fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, 0600);
  if (fileHandle.fd == -1) return PF_UNIX;

  rc = ReadFileHdr(fileHandle);
  if (rc != OK)
  {
    if (close(fd) == -1) return PF_UNIX;

    return rc;
  }

  fileHandle.bufferPool = this->bufferPool;
  fileHandle.isOpen = true;

  return OK;
}

RC PF_Manager::CloseFile(PF_FileHanlde& fileHandle);
{
  RC rc;

  if (!fileHandle.isOpen) return PF_CLOSEDFILE;

  if (close(fileHandle.fd) == -1) return PF_UNIX;
  
  fileHandle.bufferPool = NULL;
  fileHandle.isOpen = false;

  return OK;
}

RC PF_Manager::AllocateBlock(char*& buffer);
{
  return this->bufferPool->AllocateBlock(buffer);
}

RC PF_Manager::DisposeBlock(char* buffer);
{
  return this->bufferPool->DisposeBlock(buffer);
}

RC PF_Manager::InitFileHdr(int fd)
{
  PF_FileHdr fileHdr;
  fileHdr.numPages = 0;
  fileHdr.firstFree = 1;

  ssize_t writtenBytes = write(fd, &fileHdr, sizeof(fileHdr));
  
  if (writtenBytes < 0) return PF_UNIX;

  if (writtenBytes < sizeof(fileHdr)) return PF_HDRWRITE;

  return OK;
}

RC PF_Manager::ReadFileHdr(PF_FileHandle& fileHandle)
{
  ssize_t readBytes = read(fileHandle.fd, &fileHandle.fileHdr, sizeof(fileHandle.fileHdr));
  
  if (readBytes < 0) return PF_UNIX;

  if (readBytes < sizeof(fileHandle.fileHdr)) return PF_HDRREAD;

  return OK;
}
