#include "internal.h"

namespace redbase {
namespace rm {

int PageMetaData::FindEmptySlot() {
  if (IsFull()) {
    return -1;
  }

  std::vector<bool>::size_type i;
  for (i = 0; i < slots_.size(); i++) {
    if (slots_[i] == false) {
      return i;
    }
  }

  return -1;
}

void PageMetaData::Unmarshal(const char *pData) {
  std::memcpy(&next_free_, pData, sizeof(pf::PageNum));
  pData = pData + sizeof(pf::PageNum);
  for (int i = 0; i < static_cast<int>(slots_.size()); i++) {
    slots_[i] = (pData[i / 8] >> (i % 8)) & 0x1;
  }
}

void PageMetaData::Marshal(char *pData) {
  std::memcpy(pData, &next_free_, sizeof(pf::PageNum));
  pData = pData + sizeof(pf::PageNum);

  for (int i = 0; i < static_cast<int>(slots_.size()); i++) {
    if (slots_[i]) {
      pData[i / 8] |= (1 << (i % 8));
    } else {
      pData[i / 8] &= ~(1 << (i % 8));
    }
  }
}

} // namespace rm
} // namespace redbase
