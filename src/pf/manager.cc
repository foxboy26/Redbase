#include "manager.h"

#include <memory>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "glog/logging.h"
#include "src/pf/file_handle.h"

#define PF_BUFFER_SIZE 40

namespace redbase {
namespace pf {
Manager::Manager() : bufferPool_(new BufferPool(PF_BUFFER_SIZE)) {}

RC Manager::CreateFile(const char *filename) {
  int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0600);
  if (fd == -1) {
    LOG(ERROR) << "Open file failed";
    return RC::PF_UNIX;
  }

  FileHeader header;

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

RC Manager::DestroyFile(const char *filename) {
  if (remove(filename) == -1) {
    return RC::PF_UNIX;
  }
  return RC::OK;
}

RC Manager::AllocateBlock(char *&buffer) {
  return bufferPool_->AllocateBlock(buffer);
}

RC Manager::DisposeBlock(char *buffer) {
  return bufferPool_->DisposeBlock(buffer);
}
} // namespace pf
} // namespace redbase
