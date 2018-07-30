#include "file_handle.h"

#include "glog/logging.h"
#include "src/pf/page_handle.h"

namespace redbase {
namespace rm {
FileHandle::FileHandle()
    : header_(-1 /* record_size */, -1 /* num_slots */),
      is_header_modified_(false) {}

RC FileHandle::GetRec(const RID &rid, Record *rec) const {
  RC rc;
  pf::PageHandle page;
  rc = pf_file_handle_->GetThisPage(rid.GetPageNum(), &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pData;
  page.GetData(pData);
  // get page header (bitmap)
  PageMetaData meta(header_.record_size);
  meta.Unmarshal(pData);
  if (!meta.GetSlot(rid.GetSlotNum())) {
    return RC::RM_RECORDNOTFOUND;
  }
  // read actual record.
  int slotOffset = meta.ComputeSlotOffset(rid.GetSlotNum());
  rec->Init(rid, pData + slotOffset, header_.record_size);

  return pf_file_handle_->UnpinPage(rid.GetPageNum());
}

RC FileHandle::InsertRec(const char *pData, RID *rid) {
  RC rc;

  // Allodate a new page if all exist pages are full.
  pf::PageHandle page;
  if (header_.next_free == -1) {
    rc = pf_file_handle_->AllocatePage(&page);
    if (rc != RC::OK) {
      return rc;
    }
    header_.next_free = page.GetPageNum();
    is_header_modified_ = true;
  }

  // Load the page with available slots.
  const pf::PageNum page_num = header_.next_free;
  rc = pf_file_handle_->GetThisPage(page_num, &page);
  if (rc != RC::OK) {
    LOG(ERROR) << "no available page "
               << "page_num=" << page_num;
    return rc;
  }

  char *pPageData;
  page.GetData(pPageData);
  // get page header (bitmap)
  PageMetaData meta(header_.record_size);
  meta.Unmarshal(pData);

  int slot_num = meta.FindEmptySlot();
  if (slot_num == -1) {
    LOG(ERROR) << "no free slot";
    return RC::RM_RECORDNOTFOUND;
  }
  meta.SetSlot(slot_num);
  *rid = RID(page_num, slot_num);
  // copy pData to the page.
  int slotOffset = meta.ComputeSlotOffset(slot_num);
  std::memcpy(pPageData + slotOffset, pData, header_.record_size);
  // update page metadata
  meta.Marshal(pPageData);
  // if page is full, allocate and init a new page,
  if (meta.IsFull()) {
    header_.next_free = meta.NextFree();
    is_header_modified_ = true;
  }

  pf_file_handle_->MarkDirty(page_num);
  return pf_file_handle_->UnpinPage(page_num);
}

RC FileHandle::DeleteRec(const RID &rid) {
  RC rc;

  // Load the page by page_num
  pf::PageHandle page;
  pf::PageNum pageNum = rid.GetPageNum();
  SlotNum slot_num = rid.GetSlotNum();
  rc = pf_file_handle_->GetThisPage(pageNum, &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pData;
  page.GetData(pData);

  // Fetch page header (bitmap)
  PageMetaData meta(header_.record_size);
  meta.Unmarshal(pData);
  if (!meta.GetSlot(slot_num)) {
    LOG(WARNING) << "slot " << slot_num << " is already mark as deleted";
    return RC::OK;
  }

  // Mark the slot to be deleted. Also update the next_free if the page is full
  // already.
  if (meta.IsFull()) {
    meta.SetNextFree(header_.next_free);
    header_.next_free = pageNum;
    is_header_modified_ = true;
  }
  meta.UnsetSlot(slot_num);
  meta.Marshal(pData);

  pf_file_handle_->MarkDirty(rid.GetPageNum());
  return pf_file_handle_->UnpinPage(rid.GetPageNum());
}

RC FileHandle::UpdateRec(const Record &rec) {
  RID rid;
  RC rc = rec.GetRid(&rid);
  if (rc != RC::OK) {
    return rc;
  }

  // Load the page by page_num
  pf::PageNum pageNum = rid.GetPageNum();
  pf::PageHandle page;
  rc = pf_file_handle_->GetThisPage(pageNum, &page);
  if (rc != RC::OK) {
    return rc;
  }

  char *pData;
  page.GetData(pData);

  // Fetch page header (bitmap)
  PageMetaData meta(header_.record_size);
  meta.Unmarshal(pData);

  // Locate the record based on slot_num.
  if (!meta.GetSlot(rid.GetSlotNum())) {
    return RC::RM_RECORDNOTFOUND;
  }
  // update the record, by replacing the content with |rec|.
  int slotOffset = meta.ComputeSlotOffset(rid.GetSlotNum());
  std::memcpy(pData + slotOffset, rec.pData_, header_.record_size);

  pf_file_handle_->MarkDirty(pageNum);
  return pf_file_handle_->UnpinPage(pageNum);
}

} // namespace rm
} // namespace redbase
