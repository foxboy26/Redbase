#ifndef RM_RECORD_H
#define RM_RECORD_H

#include <cstring>

#include "src/pf/pf.h"
#include "src/rc.h"
#include "src/rm/rm.h"
#include "gtest/gtest_prod.h"

namespace redbase {
namespace rm {

// A record identifier uniquely identifies a record within a given file, based
// on the record's page number in the file and slot number within that page.
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

bool operator==(const RID &lhs, const RID &rhs) {
  return lhs.GetPageNum() == rhs.GetPageNum() &&
         lhs.GetSlotNum() == rhs.GetSlotNum();
}

// The Record class defines record objects.
class Record {
public:
  Record() : rid_(), pData_(nullptr){};
  Record(const Record &r) = delete;
  Record &operator=(const Record &r) = delete;

  ~Record() {
    if (pData_) {
      delete[] pData_;
      pData_ = nullptr;
    }
  };

  // Set pData to point to the record's contents
  RC GetData(char *&pData) const;

  // Get the record id
  RC GetRid(RID *rid) const;

private:
  void Init(const RID &rid, char *data, int size) {
    rid_ = rid;
    pData_ = new char[size];
    std::memcpy(pData_, data, size);
  }

  RID rid_;
  char *pData_;

  friend class FileHandle;
  // FRIEND_TEST(RM_RecordTest, Get);
  FRIEND_TEST(RM_FileHandleTest, Update);
};

} // namespace rm
} // namespace redbase

#endif // RM_RECORD_H
