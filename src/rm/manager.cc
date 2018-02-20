#include "manager.h"

#include "src/pf/file_handle.h"
#include "src/rm/internal.h"

namespace redbase {
namespace rm {
Manager::Manager(pf::Manager *pfm) : pf_manager_(pfm) {}

RC Manager::CreateFile(const char *filename, int recordSize) {
  if (recordSize >= pf::PAGE_SIZE) {
    return RC::RM_INVALIDRECORDSIZE;
  }
  RC rc = pf_manager_->CreateFile(filename);
  if (rc != RC::OK) {
    return rc;
  }

  pf::FileHandle pfFileHandle(pf_buffer_pool_);
  pf::PageHandle page;
  pfFileHandle.AllocatePage(&page);
  PageNum pageNum = page.GetPageNum();
  char *pData;
  page.GetData(pData);
  HeaderPage header(recordSize);
  header.Serialize(pData);
  pfFileHandle.MarkDirty(pageNum);
  pfFileHandle.UnpinPage(pageNum);

  pfFileHandle.Close();

  return RC::OK;
}

RC Manager::DestroyFile(const char *filename) {
  return pf_manager_->DestroyFile(filename);
}
} // namespace rm
} // namespace redbase
