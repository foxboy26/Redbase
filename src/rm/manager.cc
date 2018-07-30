#include "manager.h"

#include "absl/memory/memory.h"

#include "src/pf/file_handle.h"
#include "src/rm/internal.h"

namespace redbase {
namespace rm {
Manager::Manager(pf::Manager *pfm) : pf_manager_(pfm) {}

RC Manager::CreateFile(absl::string_view file_name, int record_size) {
  if (record_size >= pf::PAGE_SIZE) {
    return RC::RM_INVALIDRECORDSIZE;
  }
  RC rc = pf_manager_->CreateFile(file_name);
  if (rc != RC::OK) {
    return rc;
  }

  pf::FileHandle pf_file_handle;
  rc = pf_manager_->OpenFile(file_name, &pf_file_handle);
  if (rc != RC::OK) {
    return rc;
  }
  pf::PageHandle page;
  pf_file_handle.AllocatePage(&page);
  pf::PageNum pageNum = page.GetPageNum();
  char *pData;
  page.GetData(pData);

  PageMetaData meta_data(record_size);
  HeaderPage header(record_size, meta_data.NumSlots());
  header.Marshal(pData);
  pf_file_handle.MarkDirty(pageNum);
  pf_file_handle.UnpinPage(pageNum);

  pf_manager_->CloseFile(&pf_file_handle);

  return RC::OK;
}

RC Manager::DestroyFile(absl::string_view file_name) {
  return pf_manager_->DestroyFile(file_name);
}

RC Manager::OpenFile(absl::string_view file_name, FileHandle *file_handle) {
  auto pf_file_handle = absl::make_unique<pf::FileHandle>();
  RC rc = pf_manager_->OpenFile(file_name, pf_file_handle.get());
  if (rc != RC::OK) {
    return rc;
  }
  if (rc != RC::OK) {
    LOG(ERROR) << "Fail to open file: " << file_name;
    return rc;
  }

  pf::PageHandle page;
  rc = pf_file_handle->GetThisPage(kHeaderPageNum, &page);
  if (rc != RC::OK) {
    LOG(ERROR) << "Failed to get header page";
    return rc;
  }
  char *pData;
  page.GetData(pData);
  rc = pf_file_handle->UnpinPage(kHeaderPageNum);
  if (rc != RC::OK) {
    return rc;
  }

  // Init file_handle.
  file_handle->header_.Unmarshal(pData);
  file_handle->pf_file_handle_ = std::move(pf_file_handle);
  return RC::OK;
}

RC Manager::CloseFile(FileHandle *file_handle) {
  // Flush header page to disk if changed.
  if (file_handle->is_header_modified_) {
    pf::PageHandle hdr_page;
    RC rc =
        file_handle->pf_file_handle_->GetThisPage(kHeaderPageNum, &hdr_page);
    if (rc != RC::OK) {
      LOG(ERROR) << "Failed to get header page";
      return rc;
    }
    char *pData;
    hdr_page.GetData(pData);
    file_handle->header_.Marshal(pData);
    file_handle->pf_file_handle_->MarkDirty(kHeaderPageNum);
    file_handle->pf_file_handle_->UnpinPage(kHeaderPageNum);
    file_handle->pf_file_handle_->ForcePages(kHeaderPageNum);
  }

  return pf_manager_->CloseFile(file_handle->pf_file_handle_.get());
}

} // namespace rm
} // namespace redbase
