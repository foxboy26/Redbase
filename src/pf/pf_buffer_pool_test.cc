#include "pf_buffer_pool.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "googletest/include/gtest/gtest.h"

namespace {
TEST(PF_BufferPageTest, Basic) {
  PF_BufferPage page(1 /* fd */, 1 /* page num */);

  // check MarkDirty()
  page.MarkDirty();
  EXPECT_TRUE(page.IsDirty());

  // check Pin/Unpin
  EXPECT_FALSE(page.IsPinned());
  page.Pin();
  EXPECT_TRUE(page.IsPinned());
  page.Unpin();
  EXPECT_FALSE(page.IsPinned());
}

TEST(PF_BufferPageTest, ReadWriteNormal) {
  int fd = open("test", O_RDWR | O_CREAT, 0600);
  ASSERT_TRUE(fd >= 0);
  char data[] = "abcdedfg";
  ssize_t writeBytes = write(fd, data, sizeof(data));
  ASSERT_TRUE(writeBytes == sizeof(data));
  ASSERT_TRUE(close(fd) == 0);

  fd = open("test", O_RDWR, 0600);
  PF_BufferPage page(fd, 1 /* page num */);
  EXPECT_EQ(page.Read(), RC::OK);
  EXPECT_EQ(page.Data(), "abcdefg");
  ASSERT_TRUE(close(fd) == 0);
}

TEST(PF_BufferPageTest, WritePagePinned) {
  PF_BufferPage page(1 /* fd */, 1 /* page num */);
  page.MarkDirty();
  page.Pin();
  EXPECT_EQ(page.WriteBack(), RC::PF_PAGEPINNED);

  page.Unpin();
  // EXPECT_EQ(page.WriteBack(), RC::OK);
}
} // namespace
