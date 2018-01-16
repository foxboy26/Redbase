#include "pf_buffer_pool.h"

#include "googletest/include/gtest/gtest.h"

namespace {
TEST(PF_BufferPageTest, Basic) {
  PF_BufferPage page(1, 1);
  page.MarkDirty();
  page.Pin();
  EXPECT_TRUE(page.IsPinned());
  EXPECT_TRUE(page.IsDirty());

  page.Unpin();
  EXPECT_FALSE(page.IsPinned());
}

TEST(PF_BufferPageTest, ReadWriteNormal) {}

TEST(PF_BufferPageTest, WritePagePinned) {
  PF_BufferPage page(1 /* fd */, 1 /* page num */);
  page.MarkDirty();
  page.Pin();
  EXPECT_EQ(page.WriteBack(), RC::PF_PAGEPINNED);

  page.Unpin();
  // EXPECT_EQ(page.WriteBack(), RC::OK);
}
} // namespace
