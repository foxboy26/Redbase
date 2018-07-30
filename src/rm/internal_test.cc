#include "internal.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
