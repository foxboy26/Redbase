#include <fstream>
#include <iostream>

#include "src/pf/pf_file_handle.h"
#include "src/pf/pf_manager.h"
#include "src/rc.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

int main(int argc, char *argv[]) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  gflags::SetVersionString("0.1");
  std::cout << "Welcome to Redbase" << std::endl;

  PF_Manager pfm;
  RC rc = pfm.CreateFile("/home/zhiheng/test.rdb");
  if (rc != RC::OK) {
    PF_PrintError(rc);
    return -1;
  }

  std::cout << "trying to open file\n";
  PF_FileHandle pf_fh;
  rc = pf_fh.Open("test.rdb");
  if (rc != RC::OK) {
    PF_PrintError(rc);
  }
  std::cout << "Is open: " << pf_fh.IsOpen() << std::endl;

  std::cout << "Closing file...\n";
  rc = pf_fh.Close();
  if (rc != RC::OK) {
    PF_PrintError(rc);
  }

  std::cout << "Bye!" << std::endl;

  return 0;
}
