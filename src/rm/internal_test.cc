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

    char pData[redbase::pf::PAGE_DATA_SIZE] = {0};
    header.Marshal(pData);

    redbase::rm::PageMetaData another(8);
    another.Unmarshal(pData);
    EXPECT_EQ(another.NextFree(), 10);

    for (int i = 0; i < header.NumSlots(); i++) {
      EXPECT_TRUE(header.GetSlot(i));
    }
  }

  {
    redbase::rm::PageMetaData header(23);
    header.SetNextFree(10);
    for (int i = 0; i < header.NumSlots(); i++) {
      if (i % 3 == 0) {
        header.SetSlot(i);
      }
    }
    char pData[redbase::pf::PAGE_DATA_SIZE] = {0};
    header.Marshal(pData);

    redbase::rm::PageMetaData another(23);
    another.Unmarshal(pData);
    EXPECT_EQ(another.NextFree(), 10);
    for (int i = 0; i < header.NumSlots(); i++) {
      if (i % 3 == 0) {
        EXPECT_TRUE(header.GetSlot(i));
      } else {
        EXPECT_FALSE(header.GetSlot(i));
      }
    }
  }
}

TEST(PageMetaData, FindEmptySlot) {
  redbase::rm::PageMetaData header(8);
  header.SetSlot(0);
  header.SetSlot(3);
  header.SetSlot(5);

  EXPECT_EQ(1, header.FindEmptySlot());
  header.SetSlot(1);

  header.UnsetSlot(0);
  EXPECT_EQ(0, header.FindEmptySlot());
}
} // namespace
