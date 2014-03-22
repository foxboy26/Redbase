#include "PF_Manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

PF_Manager::PF_Manager()
{
  bufferPool = new PF_BufferPool(PF_BUFFER_SIZE);
}

PF_Manager::~PF_Manager()
{
  delete bufferPool;
}

void PF_Manager::CreateFile(const char* filename)
{
  int fd;

  fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, 0600);
  if (fd == -1) {
    throw PF_Exception(PF_Exception::RC::UNIX_ERROR);
  }

  try {
    InitFileHdr(fd);
  } catch (PF_Exception e) {
    if (close(fd) == -1) {
      throw PF_Exception(PF_Exception::RC::UNIX_ERROR);
    }

    if (remove(filename) == -1) {
      throw PF_Exception(PF_Exception::RC::UNIX_ERROR);
    }
  }

  if (close(fd) == -1) {
    throw PF_Exception(PF_Exception::RC::UNIX_ERROR);
  }
}

void PF_Manager::DestroyFile(const char* filename)
{
  if (remove(filename) == -1) {
    throw PF_Exception(PF_Exception::RC::UNIX_ERROR);
  }
}

void PF_FileHandle PF_Manager::OpenFile(const char* filename)
{
  assert(!fileHandle.isOpen);

  PF_FileHandle fileHandle;
  int fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, 0600);
  if (fd == -1) {
    throw PF_Exception(PF_Exception::UNIX_ERROR);
  }

  try {
    ReadFileHdr(fileHandle);
  } catch (PF_Exception e) {

    if (close(fd) == -1) return PF_UNIX;

    return rc;
  }

  fileHandle.bufferPool = this->bufferPool;
  fileHandle.isOpen = true;
}

void PF_Manager::CloseFile(PF_FileHandle& fileHandle)
{
  assert(fileHandle.isOpen);

  if (fileHandle.isHeadModfied) {
    fileHandle.
  }

  if (close(fileHandle.fd) == -1) return PF_UNIX;

  fileHandle.bufferPool = NULL;
  fileHandle.isOpen = false;
}

void PF_Manager::AllocateBlock(char*& buffer)
{
  return this->bufferPool->AllocateBlock(buffer);
}

void PF_Manager::DisposeBlock(char* buffer)
{
  return this->bufferPool->DisposeBlock(buffer);
}

void PF_Manager::InitFileHdr(int fd)
{
  PF_FileHeader fileHdr;
  fileHdr.numPages = 0;
  fileHdr.firstFree = 1;

  ssize_t writtenBytes = write(fd, &fileHdr, sizeof(fileHdr));

  if (writtenBytes < 0) return PF_UNIX;

  if (writtenBytes < sizeof(fileHdr)) return PF_HDRWRITE;
}

void PF_Manager::ReadFileHdr(PF_FileHandle& fileHandle)
{
  ssize_t readBytes = read(fileHandle.fd, &fileHandle.fileHdr, sizeof(fileHandle.fileHdr));

  if (readBytes < 0) return PF_UNIX;

  if (readBytes < sizeof(fileHandle.fileHdr)) return PF_HDRREAD;

  return OK;
}
