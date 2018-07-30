#include <fstream>
#include <iostream>

#include "absl/memory/memory.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "src/pf/file_handle.h"
#include "src/pf/manager.h"
#include "src/rc.h"

using redbase::RC;
using redbase::RC_Name;
using std::cerr;
using std::cout;
using std::endl;

void Init(int *argc, char ***argv) {
  // Initialize glog library.
  google::InitGoogleLogging((*argv)[0]);
  // Initialize gflag library.
  gflags::ParseCommandLineFlags(argc, argv, true /* remove_flag */);
}

int main(int argc, char *argv[]) {
  Init(&argc, &argv);
  gflags::SetVersionString("0.1");
  std::cout << "Welcome to Redbase" << std::endl;

  auto buffer_pool = absl::make_unique<redbase::pf::BufferPool>(100);
  redbase::pf::Manager pfm(std::move(buffer_pool));
  RC rc = pfm.CreateFile("/home/zhiheng/test.rdb");
  if (rc != RC::OK) {
    return -1;
  }

  cout << "trying to open file" << endl;
  redbase::pf::FileHandle pf_fh;
  pfm.OpenFile("/home/zhiheng/test.rdb", &pf_fh);
  if (rc != RC::OK) {
    cerr << "Error: " << RC_Name[rc] << endl;
  }
  cout << "Is open: " << pf_fh.IsOpen() << endl;

  cout << "Closing file...\n";
  rc = pf_fh.Close();
  if (rc != RC::OK) {
    cerr << "Error: " << RC_Name[rc] << endl;
  }

  std::cout << "Bye!" << std::endl;
  return 0;
}
