#ifndef PF_MANAGER_H
#define PF_MANAGER_H

#include <memory>

#include "src/pf/buffer_pool.h"
#include "src/pf/pf.h"
#include "src/rc.h"

namespace redbase {
namespace pf {
class Manager {
public:
  Manager();
  ~Manager() = default;
  RC CreateFile(const char *filename);
  RC DestroyFile(const char *filename);
  RC AllocateBlock(char *&buffer);
  RC DisposeBlock(char *buffer);

private:
  std::unique_ptr<BufferPool> bufferPool_;
};
} // namespace pf
} // namespace redbase

#endif // PF_MANAGER;
