#ifndef COMMON_H
#define COMMON_H
namespace redbase {

constexpr int MAX_STRING_LEN = 255;

enum class CompOp {
  EQ_OP,
  LT_OP,
  GT_OP,
  LE_OP,
  GE_OP,
  NE_OP,
  NO_OP,
};

enum class AttrType { INT, FLOAT, STRING };

enum class ClientHint { NO_HINT };
}; // namespace redbase

#endif // COMMON_H
