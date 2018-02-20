#ifndef RM_RECORD_H
#define RM_RECORD_H

#include "src/pf/pf.h"
#include "src/rc.h"
#include "src/rm/rm.h"

namespace redbase {
namespace rm {
class RID {
public:
  explicit RID(pf::PageNum pageNum, SlotNum slotNum)
      : pageNum_(pageNum), slotNum_(slotNum) {}
  ~RID() = default; // Destructor
  void SetPageNum(pf::PageNum pageNum) { pageNum_ = pageNum; }
  void SetSlotNum(SlotNum slotNum) { slotNum_ = slotNum; }
  pf::PageNum GetPageNum() const { return pageNum_; } // Return page number
  SlotNum GetSlotNum() const { return slotNum_; }     // Return slot number
private:
  pf::PageNum pageNum_;
  SlotNum slotNum_;
};

class Record {
public:
  Record();  // Constructor
  ~Record(); // Destructor

  RC GetData(char *&pData) const; // Set pData to point to
                                  //   the record's contents
  RC GetRid(RID *rid) const;      // Get the record id
                                  // private:
  RID rid;
  char *pData;
};
} // namespace rm
} // namespace redbase

#endif // RM_RECORD_H
