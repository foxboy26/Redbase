#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include <cstring>
#include <string>

#include "glog/logging.h"

#include "src/pf/internal.h"

namespace redbase {
namespace rm {

int ComputeSlots(int recordSize, int pageSize);

// HeaderPage contains meta data about a file.
struct HeaderPage {
  int recordSize;
  pf::PageNum nextFree;
  int numSlots;

  HeaderPage(int rSize)
      : recordSize(rSize), nextFree(1),
        numSlots(ComputeSlots(rSize, pf::PAGE_SIZE)) {}

  void Marshal(char *pData) { std::memcpy(this, pData, sizeof(HeaderPage)); }
  void Unmarshal(char *pData) { std::memcpy(pData, this, sizeof(HeaderPage)); }
};

/*
std::string charStr(char c) {
  std::string s;
  for (int i = 0; i < 8; i++) {
    s.push_back(((c >> (8 - i - 1)) & 0x1) + '0');
  }
  return s;
}*/

class PageMetaData {
public:
  explicit PageMetaData(int numSlots)
      : nextFree_(-1), numSlots_(numSlots), usedSlots_(0),
        slots_(numSlots, false) {}

  int FindEmptySlot();

  bool IsFull() { return usedSlots_ == numSlots_; }

  void Unmarshal(const char *pData);

  void Marshal(char *pData);

  int NumSlots() { return slots_.size(); }

  bool GetSlot(int slotNum) {
    if (slotNum >= numSlots_) {
      return false;
    }
    return slots_[slotNum];
  }

  void SetSlot(int slotNum) {
    if (slotNum >= numSlots_) {
      return;
    }

    slots_[slotNum] = true;
    ++usedSlots_;
  }
  void UnsetSlot(int slotNum) {
    if (slotNum >= numSlots_) {
      return;
    }

    slots_[slotNum] = false;
    --usedSlots_;
  }

  int ComputeSlotOffset(int recordSize, int slotNum) {
    // slotsSize = ceiling(numSlots_ / 8) since 1 slot need 1 bit.
    int bitmapBytes = (numSlots_ + 8 - 1) / 8;
    return sizeof(nextFree_) + bitmapBytes + recordSize * slotNum;
  }

  std::string FreeSlotsString() {
    std::string str;
    for (const auto &s : slots_) {
      str.push_back(char(s + '0'));
    }
    return str;
  }

  void SetNextFree(const pf::PageNum pageNum) { nextFree_ = pageNum; }
  pf::PageNum NextFree() { return nextFree_; }

private:
  pf::PageNum nextFree_;
  const int numSlots_;
  int usedSlots_;
  std::vector<bool> slots_;
};

constexpr int HEADER_PAGE_SIZE = sizeof(HeaderPage);

int ComputeSlots(int recordSize, int pageSize) {
  int availBytes = pageSize - HEADER_PAGE_SIZE;
  // each record requires one extra bit in bitmap, that's 1/8 bytes.
  return static_cast<int>(availBytes / (recordSize + 0.125));
}

} // namespace rm
} // namespace redbase

#endif // RM_INTERNAL_H
