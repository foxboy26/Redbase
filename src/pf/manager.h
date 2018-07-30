#ifndef PF_MANAGER_H
#define PF_MANAGER_H

#include <memory>

#include "absl/strings/string_view.h"

#include "src/pf/buffer_pool.h"
#include "src/pf/file_handle.h"
#include "src/pf/pf.h"
#include "src/rc.h"

namespace redbase {
namespace pf {
class Manager {
public:
  explicit Manager(std::unique_ptr<BufferPool> buffer_pool);
  ~Manager() = default;
  RC CreateFile(absl::string_view file_name);
  RC DestroyFile(absl::string_view file_name);
  RC OpenFile(absl::string_view file_name, FileHandle *file_handle);
  RC CloseFile(FileHandle *file_handle);
  RC AllocateBlock(char *&buffer);
  RC DisposeBlock(char *buffer);

private:
  std::unique_ptr<BufferPool> buffer_pool_;
};
} // namespace pf
} // namespace redbase

#endif // PF_MANAGER;
