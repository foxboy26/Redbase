#ifndef RM_FILE_SCAN
#define RM_FILE_SCAN

#include "src/common.h"
#include "src/rm/file_handle.h"
#include "src/rm/record.h"

namespace redbase {
namespace rm {

// The RM_FileScan class provides clients the capability to perform scans over
// the records of an RM component file, where a scan may be based on a specified
// condition.
class FileScan {
public:
  FileScan();
  ~FileScan();
  // Initialize file scan
  RC OpenScan(FileHandle *fileHandle, AttrType attr_type, int attr_len,
              int attr_offset, CompOp comp_op, void *value,
              ClientHint pinHint = ClientHint::NO_HINT);
  RC GetNextRec(Record *rec); // Get next matching record
  RC CloseScan();             // Terminate file scan
private:
  int GetIntVal(char *data, int offset) {
    int v;
    std::memcpy(&v, data + offset, sizeof(int));
    return v;
  }
  float GetFloatVal(char *data, int offset) {
    float v;
    std::memcpy(&v, data + offset, sizeof(float));
    return v;
  }

  std::string GetStrVal(char *data, int offset, int len) {
    char v[255];
    std::memcpy(v, data + offset, len);
    return std::string(v);
  }

  template <typename T> bool Compare(T lhs, T rhs, CompOp op) {
    switch (op) {
    case CompOp::EQ_OP:
      return lhs == rhs;
    case CompOp::GE_OP:
      return lhs >= rhs;
    case CompOp::GT_OP:
      return lhs > rhs;
    case CompOp::LE_OP:
      return lhs <= rhs;
    case CompOp::LT_OP:
      return lhs < rhs;
    case CompOp::NE_OP:
      return lhs != rhs;
    case CompOp::NO_OP:
      return true;
    default:
      return false;
    }
  }

  bool Filter(char *data) {
    int int_val;
    float float_val;
    std::string str_val;
    switch (attr_type_) {
    case AttrType::INT:
      int_val = GetIntVal(data, attr_offset_);
      return Compare<int>(int_val, int_val_, comp_op_);
    case AttrType::FLOAT:
      float_val = GetIntVal(data, attr_offset_);
      return Compare<float>(float_val, float_val_, comp_op_);
    case AttrType::STRING:
      str_val = GetStrVal(data, attr_offset_, attr_len_);
      return Compare<std::string>(str_val, std::string(str_val_), comp_op_);
    default:
      return false;
    }
  }

  RID cur_rid_;

  AttrType attr_type_;
  int attr_len_;
  int attr_offset_;
  CompOp comp_op_;
  int int_val_;
  float float_val_;
  char str_val_[255];
  ClientHint hint_;

  FileHandle *file_handle_; // not owned.
};

} // namespace rm
} // namespace redbase

#endif // RM_FILE_SCAN
