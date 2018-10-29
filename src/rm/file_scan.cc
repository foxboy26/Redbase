#include "file_scan.h"

#include <functional>

namespace redbase {
namespace rm {

FileScan::FileScan() : cur_rid_(1, 0), file_handle_(nullptr) {}

FileScan::~FileScan() {}

RC FileScan::OpenScan(FileHandle *file_handle, AttrType attr_type, int attr_len,
                      int attr_offset, CompOp comp_op, void *value,
                      ClientHint hint) {
  // Check if there's an existing scan ongoing.
  if (file_handle_) {
    return RC::RM_RECORDNOTFOUND;
  }

  file_handle_ = file_handle;

  switch (attr_type) {
  case AttrType::INT:
    int_val_ = *(reinterpret_cast<int *>(value));
    break;
  case AttrType::FLOAT:
    float_val_ = *(reinterpret_cast<float *>(value));
    break;
  case AttrType::STRING:
    // TODO(zhiheng) check if attr_len < 255
    std::memcpy(str_val_, value, attr_len);
    break;
  default:
    return RC::RM_RECORDNOTFOUND;
  }

  attr_type_ = attr_type;
  attr_len_ = attr_len;
  attr_offset_ = attr_offset;
  comp_op_ = comp_op;
  hint_ = hint;

  return RC::OK;
}

RC FileScan::GetNextRec(Record *record) {
  using namespace std::placeholders;
  RC rc;
  rc = file_handle_->GetNextRec(cur_rid_, record,
                                std::bind(&FileScan::Filter, this, _1));
  if (rc == RC::PF_EOF) {
    return rc;
  }
  if (rc == RC::OK) {
    record->GetRid(&cur_rid_);
    return RC::OK;
  } else {
    return rc;
  }
}

RC FileScan::CloseScan() {
  file_handle_ = nullptr;
  return RC::OK;
}

} // namespace rm
} // namespace redbase
