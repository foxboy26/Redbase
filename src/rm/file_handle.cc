#include "file_handle.h"
#include "src/pf/page_handle.h"

namespace {} // namespace

namespace redbase {
namespace rm {
FileHandle::FileHandle() : header_(-1), isHeaderModified_(false) {}

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
  std::memcpy(rec->pData, pData + slotOffset, header_.recordSize);
  rec->rid = rid;

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
  rid->SetPageNum(header_.nextFree);
  rid->SetSlotNum(slotNum);
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
  RID rid = rec.rid;
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
  std::memcpy(pData + slotOffset, rec.pData, header_.recordSize);

  pfFileHandle_->MarkDirty(pageNum);
  return pfFileHandle_->UnpinPage(pageNum);
}

} // namespace rm
} // namespace redbase
