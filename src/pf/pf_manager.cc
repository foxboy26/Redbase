#include "pf_manager.h"

#include <memory>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"
#include "src/pf/pf_file_handle.h"

#define PF_BUFFER_SIZE 40

PF_Manager::PF_Manager() : bufferPool_(new PF_BufferPool(PF_BUFFER_SIZE)) {}

RC PF_Manager::CreateFile(const char *filename) {
  int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0600);
  if (fd == -1) {
    LOG(ERROR) << "Open file failed";
    return RC::PF_UNIX;
  }

  PF_FileHeader header;

  ssize_t writtenBytes = write(fd, &header, sizeof(header));

  if (writtenBytes < 0) {
    LOG(ERROR) << "Write header to file failed";
    return RC::PF_UNIX;
  }

  if (writtenBytes < static_cast<ssize_t>(sizeof(header))) {
    LOG(ERROR) << "Write incompleted header to the file: " << filename;
    return RC::PF_HDRWRITE;
  }

  if (close(fd) == -1) {
    LOG(ERROR) << "Failed to close file: " << filename;
    return RC::PF_UNIX;
  }

  return RC::OK;
}

RC PF_Manager::DestroyFile(const char *filename) {
  if (remove(filename) == -1) {
    return RC::PF_UNIX;
  }
  return RC::OK;
}

RC PF_Manager::AllocateBlock(char *&buffer) {
  return bufferPool_->AllocateBlock(buffer);
}

RC PF_Manager::DisposeBlock(char *buffer) {
  return bufferPool_->DisposeBlock(buffer);
}
