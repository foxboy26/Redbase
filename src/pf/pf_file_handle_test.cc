#include "pf_file_handle.h"
#include "pf_manager.h"

#include "googletest/include/gtest/gtest.h"
namespace {
TEST(PF_FileHandleTest, OpenFile) {
  PF_FileHandle fh;
  EXPECT_FALSE(fh.IsOpen());

  PF_Manager pfm;
  pfm.CreateFile("test.rdb");

  fh.Open("test.rdb");
  EXPECT_TRUE(fh.IsOpen());

  // Close the file.
  fh.Close();
  EXPECT_FALSE(fh.IsOpen());
}

TEST(PF_FileHandleTest, OpenNonExistFile) {
  PF_FileHandle fh;
  EXPECT_FALSE(fh.IsOpen());
  fh.Open("non_exist.rdb");
  EXPECT_FALSE(fh.IsOpen());
}

} // namespace
