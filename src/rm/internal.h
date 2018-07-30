#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include <cstring>
#include <string>

#include "glog/logging.h"

#include "src/pf/internal.h"

namespace redbase {
namespace rm {

constexpr pf::PageNum kHeaderPageNum = 0;

// HeaderPage contains meta data about a file.
struct HeaderPage {
  int record_size;       // size of each record stored in the file.
  pf::PageNum next_free; // points to the next page with free slots.
  int num_slots;         // total number of slots available in one page.

  HeaderPage(int record_size, int num_slots)
      : record_size(record_size), next_free(-1), num_slots(num_slots) {}

  void Marshal(char *pData) { std::memcpy(this, pData, sizeof(HeaderPage)); }
  void Unmarshal(char *pData) { std::memcpy(pData, this, sizeof(HeaderPage)); }
};

constexpr int HEADER_PAGE_SIZE = sizeof(HeaderPage);

// PageMetaData contains meta data about each page in a file.
class PageMetaData {
public:
  explicit PageMetaData(int record_size)
      : next_free_(-1), record_size_(record_size),
        num_slots_(ComputeNumSlots(pf::PAGE_DATA_SIZE, record_size)),
        header_size_(HeaderSize()), usedSlots_(0), slots_(num_slots_, false) {}

  static int HeaderSize(const int num_slots) {
    return sizeof(next_free_) + (num_slots + 8 - 1) / 8;
  }

  // Returns the next free slot within this page by doing a linear scan on the
  // bitmap. -1 indicates no slot is available.
  int FindEmptySlot();

  bool IsFull() const { return usedSlots_ == num_slots_; }

  void Unmarshal(const char *pData);

  void Marshal(char *pData);

  int NumSlots() { return num_slots_; }

  bool GetSlot(int slotNum) {
    if (slotNum >= num_slots_) {
      return false;
    }
    return slots_[slotNum];
  }

  void SetSlot(int slotNum) {
    if (slotNum >= num_slots_) {
      return;
    }

    slots_[slotNum] = true;
    ++usedSlots_;
  }
  void UnsetSlot(int slotNum) {
    if (slotNum >= num_slots_) {
      return;
    }

    slots_[slotNum] = false;
    --usedSlots_;
  }

  int ComputeSlotOffset(int slotNum) {
    return HeaderSize() + record_size_ * slotNum;
  }

  std::string FreeSlotsString() {
    std::string str;
    for (const auto &s : slots_) {
      str.push_back(char(s + '0'));
    }
    return str;
  }

  void SetNextFree(const pf::PageNum pageNum) { next_free_ = pageNum; }
  pf::PageNum NextFree() { return next_free_; }

private:
  // Each page has a meta-data header, which is next_free + a bitmap.
  // Each record will require 1 bit ( 1/8 byte) in the bitmap.
  int ComputeNumSlots(int page_size, int record_size) {
    return 8 * (page_size - sizeof(next_free_)) / (8 * record_size + 1);
  }

  // The bitmap needs ceiling(num_slots_ / 8) bytes.
  int HeaderSize() { return sizeof(next_free_) + (num_slots_ + 8 - 1) / 8; }

  pf::PageNum next_free_;
  const int record_size_;
  const int num_slots_;
  const int header_size_;
  int usedSlots_;
  std::vector<bool> slots_;
};
} // namespace rm
} // namespace redbase

#endif // RM_INTERNAL_H
