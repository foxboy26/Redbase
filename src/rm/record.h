#ifndef RM_RECORD_H
#define RM_RECORD_H

//#include "googletest/include/gtest/gtest_prod.h"
#include "src/pf/pf.h"
#include "src/rc.h"
#include "src/rm/rm.h"
#include "gtest/gtest_prod.h"

namespace redbase {
namespace rm {
class RID {
public:
  RID() : pageNum_(redbase::pf::kInvalidPageNum), slotNum_(kInvalidSlotNum) {}
  explicit RID(pf::PageNum pageNum, SlotNum slotNum)
      : pageNum_(pageNum), slotNum_(slotNum) {}

  RID(const RID &rid) = default;
  RID &operator=(const RID &rid) = default;
  ~RID() = default; // Destructor

  bool IsValid() const {
    return pageNum_ != redbase::pf::kInvalidPageNum &&
           slotNum_ != kInvalidSlotNum;
  }

  pf::PageNum GetPageNum() const { return pageNum_; } // Return page number
  SlotNum GetSlotNum() const { return slotNum_; }     // Return slot number
private:
  pf::PageNum pageNum_;
  SlotNum slotNum_;
};

class Record {
public:
  Record() : rid_(), pData_(nullptr){};

  ~Record() = default;

  // Set pData to point to the record's contents
  RC GetData(char *&pData) const;

  // Get the record id
  RC GetRid(RID *rid) const;

private:
  RID rid_;
  char *pData_;

  friend class FileHandle;
  FRIEND_TEST(RM_RecordTest, Get);
};
} // namespace rm
} // namespace redbase

#endif // RM_RECORD_H
