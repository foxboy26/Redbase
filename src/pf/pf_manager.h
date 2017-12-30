#ifndef PF_PF_MANAGER_H
#define PF_PF_MANAGER_H

#include <memory>

#include "src/pf/pf.h"
#include "src/pf/pf_buffer_pool.h"
#include "src/rc.h"

// class PF_FileHandle;

class PF_Manager {
public:
  PF_Manager();
  ~PF_Manager() = default;
  RC CreateFile(const char *filename);
  RC DestroyFile(const char *filename);
  RC AllocateBlock(char *&buffer);
  RC DisposeBlock(char *buffer);

private:
  std::unique_ptr<PF_BufferPool> bufferPool_;
};

#endif // PF_PF_MANAGER;
