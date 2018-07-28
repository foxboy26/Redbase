#include "manager.h"

#include "src/pf/file_handle.h"
#include "src/rm/internal.h"

namespace redbase {
namespace rm {
Manager::Manager(pf::BufferPool *buffer_pool, pf::Manager *pfm)
    : pfBufferPool_(buffer_pool), pfManager_(pfm) {}

RC Manager::CreateFile(const char *filename, int recordSize) {
  if (recordSize >= pf::PAGE_SIZE) {
    return RC::RM_INVALIDRECORDSIZE;
  }
  RC rc = pfManager_->CreateFile(filename);
  if (rc != RC::OK) {
    return rc;
  }

  pf::FileHandle pfFileHandle(pfBufferPool_);
  rc = pfFileHandle.Open(filename);
  if (rc != RC::OK) {
    return rc;
  }
  pf::PageHandle page;
  pfFileHandle.AllocatePage(&page);
  pf::PageNum pageNum = page.GetPageNum();
  char *pData;
  page.GetData(pData);
  HeaderPage header(recordSize);
  header.Marshal(pData);
  pfFileHandle.MarkDirty(pageNum);
  pfFileHandle.UnpinPage(pageNum);

  pfFileHandle.Close();

  return RC::OK;
}

RC Manager::DestroyFile(const char *filename) {
  return pfManager_->DestroyFile(filename);
}

} // namespace rm
} // namespace redbase
