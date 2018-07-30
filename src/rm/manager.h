#ifndef RM_MANAGER_H
#define RM_MANAGER_H

#include "absl/strings/string_view.h"

#include "src/pf/manager.h"
#include "src/rm/file_handle.h"

namespace redbase {
namespace rm {
class Manager {
public:
  explicit Manager(pf::Manager *pfm);
  ~Manager() = default;
  // Create a new file
  RC CreateFile(absl::string_view file_name, int record_size);
  // Destroy a file
  RC DestroyFile(absl::string_view file_name);

  RC OpenFile(absl::string_view file_name, FileHandle *file_handle);
  RC CloseFile(FileHandle *file_handle);

private:
  pf::Manager *pf_manager_;
};
} // namespace rm
} // namespace redbase

#endif // RM_MANAGER_H
