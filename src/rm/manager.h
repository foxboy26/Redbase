#ifndef RM_MANAGER_H
#define RM_MANAGER_H

#include "src/pf/manager.h"
#include "src/rm/file_handle.h"

namespace redbase {
namespace rm {
class Manager {
public:
  explicit Manager(pf::Manager *pfm); // Constructor
  ~Manager();                         // Destructor
  // Create a new file
  RC CreateFile(const char *fileName, int recordSize);
  RC DestroyFile(const char *fileName); // Destroy a file
  RC OpenFile(const char *fileName, FileHandle &fileHandle);
  // Open a file
  RC CloseFile(FileHandle &fileHandle); // Close a file
private:
  pf::Manager *pf_manager_;
  pf::BufferPool *pf_buffer_pool_;
};
} // namespace rm
} // namespace redbase

#endif // RM_MANAGER_H
