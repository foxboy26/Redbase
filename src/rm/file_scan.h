#ifndef RM_FILE_SCAN
#define RM_FILE_SCAN

#include "src/common.h"
#include "src/rm/file_handle.h"
#include "src/rm/record.h"

namespace redbase {
namespace rm {
class FileScan {
public:
  FileScan();                               // Constructor
  ~FileScan();                              // Destructor
  RC OpenScan(const FileHandle &fileHandle, // Initialize file scan
              AttrType attrType, int attrLength, int attrOffset, CompOp compOp,
              void *value, ClientHint pinHint = ClientHint::NO_HINT);
  RC GetNextRec(Record &rec); // Get next matching record
  RC CloseScan();             // Terminate file scan
};
} // namespace rm
} // namespace redbase

#endif // RM_FILE_SCAN
