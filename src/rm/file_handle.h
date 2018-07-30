#ifndef RM_FILE_HANDLE_H
#define RM_FILE_HANDLE_H

#include <memory>

#include "src/pf/file_handle.h"
#include "src/rc.h"
#include "src/rm/internal.h"
#include "src/rm/record.h"

namespace redbase {
namespace rm {
class FileHandle {
public:
  FileHandle();
  ~FileHandle() = default;

  // Get a record
  RC GetRec(const RID &rid, Record *rec) const;
  // Insert a new record, return record id
  RC InsertRec(const char *pData, RID *rid);
  // Delete a record
  RC DeleteRec(const RID &rid);
  // Update a record
  RC UpdateRec(const Record &rec);
  // Write dirty page(s) to disk
  RC ForcePages(pf::PageNum pageNum = pf::ALL_PAGES) const;

private:
  HeaderPage header_;
  bool is_header_modified_;
  std::unique_ptr<pf::FileHandle> pf_file_handle_;

  friend class Manager;
};
} // namespace rm
} // namespace redbase

#endif //  RM_FILE_HANDLE_H
