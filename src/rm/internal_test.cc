#include "internal.h"

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

/*namespace {
std::string charStr(char c) {
  std::string s;
  for (int i = 0; i < 8; i++) {
    s.push_back(((c >> (8 - i - 1)) & 0x1) + '0');
  }
  return s;
}
} // namespace
*/

namespace {
TEST(PageMetaData, MarshalUnMarshal) {
  {
    redbase::rm::PageMetaData header(8);
    header.SetNextFree(10);
    for (int i = 0; i < header.NumSlots(); i++) {
      header.SetSlot(i);
    }
    EXPECT_TRUE(header.IsFull());

    char pData[100] = {0};
    header.Marshal(pData);

    redbase::rm::PageMetaData another(8);
    another.Unmarshal(pData);
    EXPECT_EQ(another.NextFree(), 10);
    EXPECT_EQ("11111111", another.FreeSlotsString());
  }

  {
    redbase::rm::PageMetaData header(23);
    header.SetNextFree(10);
    for (int i = 0; i < header.NumSlots(); i++) {
      if (i % 3 == 0) {
        header.SetSlot(i);
      }
    }
    char pData[100] = {0};
    header.Marshal(pData);

    redbase::rm::PageMetaData another(23);
    another.Unmarshal(pData);
    EXPECT_EQ(another.NextFree(), 10);
    EXPECT_EQ("10010010010010010010010", another.FreeSlotsString());
  }
}
} // namespace
