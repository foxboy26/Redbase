#include "record.h"

namespace redbase {
namespace rm {

RC Record::GetData(char *&pData) const {
  if (pData_ == nullptr) {
    return RC::RM_INVALID_RECORD;
  }
  pData = pData_;
  return RC::OK;
}

RC Record::GetRid(RID *rid) const {
  if (rid_.IsValid()) {
    *rid = rid_;
    return RC::OK;
  }
  return RC::RM_INVALID_RECORD;
}

} // namespace rm
} // namespace redbase
