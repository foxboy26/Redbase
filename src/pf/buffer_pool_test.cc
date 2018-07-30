#include "buffer_pool.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "googletest/include/gtest/gtest.h"
#include "src/common/test_utils.h"

using redbase::RC;

namespace {
TEST(BufferPageTest, Basic) {
  redbase::pf::BufferPage page(1 /* fd */, 1 /* page num */);

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

TEST(BufferPageTest, ReadWriteNormal) {
  char data[] = "abcdefg";
  int fd = open("read_write_normal", O_RDWR | O_CREAT, 0600);
  ASSERT_GE(fd, 0);
  redbase::pf::BufferPage page(fd, 0 /* page num */);
  memcpy(page.Data(), data, sizeof(data));
  page.MarkDirty();
  RC rc = page.WriteBack();
  EXPECT_OK(rc) << "got rc=" << redbase::RC_Name[rc];
  EXPECT_FALSE(page.IsDirty());

  redbase::pf::BufferPage new_page(fd, 0 /* page_num */);
  EXPECT_EQ(new_page.Read(), RC::OK);
  EXPECT_STREQ(new_page.Data(), data);

  ASSERT_EQ(0, close(fd));
}

TEST(BufferPageTest, WritePagePinned) {
  int fd = open("test", O_RDWR | O_CREAT, 0600);
  ASSERT_TRUE(fd >= 0);
  redbase::pf::BufferPage page(fd, 0 /* page num */);
  page.MarkDirty();
  page.Pin();
  EXPECT_EQ(page.WriteBack(), redbase::RC::PF_PAGEPINNED);

  page.Unpin();

  RC rc = page.WriteBack();
  EXPECT_OK(rc) << "got rc=" << redbase::RC_Name[rc];
  EXPECT_FALSE(page.IsDirty());

  ASSERT_EQ(0, close(fd));
  ASSERT_EQ(0, remove("test"));
}
} // namespace
