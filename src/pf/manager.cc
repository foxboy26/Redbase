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
Manager::Manager(std::unique_ptr<BufferPool> buffer_pool)
    : buffer_pool_(std::move(buffer_pool)) {}

RC Manager::CreateFile(absl::string_view file_name) {
  int fd =
      open(std::string(file_name).c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
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
    LOG(ERROR) << "Write incompleted header to the file: " << file_name;
    return RC::PF_HDRWRITE;
  }

  if (close(fd) == -1) {
    LOG(ERROR) << "Failed to close file: " << file_name;
    return RC::PF_UNIX;
  }

  return RC::OK;
}

RC Manager::DestroyFile(absl::string_view file_name) {
  if (remove(std::string(file_name).c_str()) == -1) {
    return RC::PF_UNIX;
  }
  return RC::OK;
}

RC Manager::OpenFile(absl::string_view file_name, FileHandle *file_handle) {
  file_handle->buffer_pool_ = buffer_pool_.get();
  return file_handle->Open(file_name);
}

RC Manager::CloseFile(FileHandle *file_handle) { return file_handle->Close(); }

RC Manager::AllocateBlock(char *&buffer) {
  return buffer_pool_->AllocateBlock(buffer);
}

RC Manager::DisposeBlock(char *buffer) {
  return buffer_pool_->DisposeBlock(buffer);
}
} // namespace pf
} // namespace redbase
