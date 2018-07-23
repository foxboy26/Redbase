#include "file_handle.h"

#include "glog/logging.h"
#include "src/pf/page_handle.h"

namespace redbase {
namespace rm {
FileHandle::FileHandle() : header_(-1), isHeaderModified_(false) {}

RC FileHandle::OpenFile(const char *filename) {
  RC rc = pfFileHandle_->Open(filename);
  if (rc != RC::OK) {
    LOG(ERROR) << "Fail to open file: " << filename;
    return rc;
  }

  pf::PageHandle page;
  rc = pfFileHandle_->GetThisPage(0, &page);
  if (rc != RC::OK) {
    LOG(ERROR) << "Failed to get header page";
    return rc;
  }
  char *pData;
  page.GetData(pData);
  header_.Unmarshal(pData);
  pfFileHandle_->UnpinPage(0);

  return RC::OK;
}

RC FileHandle::CloseFile() {
  if (isHeaderModified_) {
    pf::PageHandle page;
    RC rc = pfFileHandle_->GetThisPage(0, &page);
    if (rc != RC::OK) {
      LOG(ERROR) << "Failed to get header page";
      return rc;
    }
    char *pData;
    page.GetData(pData);
    header_.Marshal(pData);
    pfFileHandle_->MarkDirty(0);
    pfFileHandle_->UnpinPage(0);
  }

  return pfFileHandle_->Close();
}

RC FileHandle::GetRec(const RID &rid, Record *rec) const {
  RC rc;
  pf::PageHandle page;
  rc = pfFileHandle_->GetThisPage(rid.GetPageNum(), &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pData;
  page.GetData(pData);
  // get page header (bitmap)
  PageMetaData meta(header_.numSlots);
  meta.Unmarshal(pData);
  if (!meta.GetSlot(rid.GetSlotNum())) {
    return RC::RM_RECORDNOTFOUND;
  }
  // read actual record.
  int slotOffset = meta.ComputeSlotOffset(header_.recordSize, rid.GetSlotNum());
  std::memcpy(rec->pData_, pData + slotOffset, header_.recordSize);
  rec->rid_ = rid;

  return pfFileHandle_->UnpinPage(rid.GetPageNum());
}

RC FileHandle::InsertRec(const char *pData, RID *rid) {
  pf::PageNum nextFree = header_.nextFree;
  RC rc;
  pf::PageHandle page;
  rc = pfFileHandle_->GetThisPage(header_.nextFree, &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pPageData;
  page.GetData(pPageData);
  // get page header (bitmap)
  PageMetaData meta(header_.numSlots);
  meta.Unmarshal(pData);

  // if page is full, allocate and init a new page,
  if (meta.IsFull()) {
    header_.nextFree = meta.NextFree();
    isHeaderModified_ = true;
  }

  int slotNum = meta.FindEmptySlot();
  meta.SetSlot(slotNum);
  *rid = RID(header_.nextFree, slotNum);
  // update page metadata
  meta.Marshal(pPageData);
  // copy pData to the page.
  int slotOffset = meta.ComputeSlotOffset(header_.recordSize, slotNum);
  std::memcpy(pPageData + slotOffset, pData, header_.recordSize);
  pfFileHandle_->MarkDirty(nextFree);
  pfFileHandle_->UnpinPage(nextFree);

  return RC::OK;
}

RC FileHandle::DeleteRec(const RID &rid) {
  RC rc;
  pf::PageHandle page;
  pf::PageNum pageNum = rid.GetPageNum();
  SlotNum slotNum = rid.GetSlotNum();
  rc = pfFileHandle_->GetThisPage(pageNum, &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pData;
  page.GetData(pData);
  // get page header (bitmap)
  PageMetaData meta(header_.numSlots);
  meta.Unmarshal(pData);
  if (!meta.GetSlot(slotNum)) {
    LOG(WARNING) << "slot " << slotNum << " is already mark as deleted";
    return RC::OK;
  }
  meta.UnsetSlot(slotNum);
  meta.Marshal(pData);

  pfFileHandle_->MarkDirty(rid.GetPageNum());
  return pfFileHandle_->UnpinPage(rid.GetPageNum());
}

RC FileHandle::UpdateRec(const Record &rec) {
  RC rc;
  pf::PageHandle page;
  RID rid = rec.rid_;
  pf::PageNum pageNum = rid.GetPageNum();
  rc = pfFileHandle_->GetThisPage(pageNum, &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pData;
  page.GetData(pData);
  // get page header (bitmap)
  PageMetaData meta(header_.numSlots);
  meta.Unmarshal(pData);
  if (!meta.GetSlot(rid.GetSlotNum())) {
    return RC::RM_RECORDNOTFOUND;
  }
  // write record.
  int slotOffset = meta.ComputeSlotOffset(header_.recordSize, rid.GetSlotNum());
  std::memcpy(pData + slotOffset, rec.pData_, header_.recordSize);

  pfFileHandle_->MarkDirty(pageNum);
  return pfFileHandle_->UnpinPage(pageNum);
}

} // namespace rm
} // namespace redbase
