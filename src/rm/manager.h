#ifndef RM_MANAGER_H
#define RM_MANAGER_H

#include "src/pf/manager.h"
#include "src/rm/file_handle.h"

namespace redbase {
namespace rm {
class Manager {
public:
  explicit Manager(pf::BufferPool *buffer_pool,
                   pf::Manager *pfm); // Constructor
  ~Manager() = default;               // Destructor
  // Create a new file
  RC CreateFile(const char *fileName, int recordSize);
  RC DestroyFile(const char *fileName); // Destroy a file
private:
  pf::BufferPool *pfBufferPool_;
  pf::Manager *pfManager_;
};
} // namespace rm
} // namespace redbase

#endif // RM_MANAGER_H
