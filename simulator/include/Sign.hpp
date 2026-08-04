#ifndef SIGN_HPP
#define SIGN_HPP
#include <compare>
#include <cstdint>

enum class Op : std::uint8_t
{
  Invalid,

  // U 型
  Lui,
  Auipc,

  // 跳转
  Jal,
  Jalr,

  // 分支
  Beq,
  Bne,
  Blt,
  Bge,
  Bltu,
  Bgeu,

  // Load
  Lb,
  Lh,
  Lw,
  Lbu,
  Lhu,

  // Store
  Sb,
  Sh,
  Sw,

  // I 型运算
  Addi,
  Slti,
  Sltiu,
  Xori,
  Ori,
  Andi,
  Slli,
  Srli,
  Srai,

  // R 型运算
  Add,
  Sub,
  Xor,
  Or,
  And,
  Sll,
  Srl,
  Sra,
  Slt,
  Sltu,
};

enum class ROBType
{
  WriteReg,
  WriteMem,
  Branch
};

struct FetchToMemSign
{
  bool pc_valid_ = false;
  uint32_t pc_ = 0;

  auto operator<=>(const FetchToMemSign &) const = default;
};

struct FetchToDecoderSign
{
  bool inst_valid_ = false;
  uint32_t inst_ = 0;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
  bool predicted_next_pc_valid_ = false;
  uint32_t predicted_next_pc_ = 0;

  auto operator<=>(const FetchToDecoderSign &) const = default;
};

struct DecoderToIssueSign
{
  bool raw_instruction_valid_ = false;
  uint32_t raw_instruction_ = 0;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
  bool predicted_next_pc_valid_ = false;
  uint32_t predicted_next_pc_ = 0;
  bool op_valid_ = false;
  Op op_ = Op::Invalid;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool rs1_valid_ = false;
  uint8_t rs1_ = 0;
  bool rs2_valid_ = false;
  uint8_t rs2_ = 0;
  bool imm_valid_ = false;
  uint32_t imm_ = 0;

  auto operator<=>(const DecoderToIssueSign &) const = default;
};

struct IssueToRATSign
{
  bool register_id_valid_ = false;
  uint8_t register_id_ = 0;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;

  auto operator<=>(const IssueToRATSign &) const = default;
};

struct IssueToLSQSign
{
  Op op_ = Op::Invalid;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool Vj_valid_ = false;
  uint32_t Vj_ = 0;
  bool Vk_valid_ = false;
  uint32_t Vk_ = 0;
  bool imm_valid_ = false;
  uint32_t imm_ = 0;
  bool Qj_valid_ = false;
  uint8_t Qj_ = 0;
  bool Qk_valid_ = false;
  uint8_t Qk_ = 0;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;

  auto operator<=>(const IssueToLSQSign &) const = default;
};

struct IssueToRSSign
{
  Op op_ = Op::Invalid;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
  bool Vj_valid_ = false;
  uint32_t Vj_ = 0;
  bool Vk_valid_ = false;
  uint32_t Vk_ = 0;
  bool imm_valid_ = false;
  uint32_t imm_ = 0;
  bool Qj_valid_ = false;
  uint8_t Qj_ = 0;
  bool Qk_valid_ = false;
  uint8_t Qk_ = 0;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;

  auto operator<=>(const IssueToRSSign &) const = default;
};

struct IssueToROBSign
{
  ROBType rob_type;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool predicted_pc_valid_ = false;
  uint32_t predicted_pc_ = 0;

  auto operator<=>(const IssueToROBSign &) const = default;
};

struct LSQToMemSign
{
  Op op_ = Op::Invalid;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool addr_valid_ = false;
  uint32_t addr_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;

  auto operator<=>(const LSQToMemSign &) const = default;
};

struct LSQToCDBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;

  auto operator<=>(const LSQToCDBSign &) const = default;
};

struct LSQToROBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool addr_valid_ = false;
  uint32_t addr_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;

  auto operator<=>(const LSQToROBSign &) const = default;
};

struct RSToALUSign
{
  Op op_ = Op::Invalid;
  bool Vj_valid_ = false;
  uint32_t Vj_ = 0;
  bool Vk_valid_ = false;
  uint32_t Vk_ = 0;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
  bool offset_valid_ = false;
  uint32_t offset_ = 0;

  auto operator<=>(const RSToALUSign &) const = default;
};

struct ALUToCDBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;

  auto operator<=>(const ALUToCDBSign &) const = default;
};

struct CDBToRS_LSQ_ROB_IssueSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;

  auto operator<=>(const CDBToRS_LSQ_ROB_IssueSign &) const = default;
};

struct MemToLSQSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
  bool is_written_ = false;

  auto operator<=>(const MemToLSQSign &) const = default;
};

struct MemToROBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool is_written_ = false;

  auto operator<=>(const MemToROBSign &) const = default;
};

struct ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
{
  bool need_flush_ = false;

  auto operator<=>(const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
                       &) const = default;
};

struct ROBToFetchFlushSign
{
  bool need_flush_ = false;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;

  auto operator<=>(const ROBToFetchFlushSign &) const = default;
};

struct ROBToLSQSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool write_enabled_ = false;

  auto operator<=>(const ROBToLSQSign &) const = default;
};

struct ROBToRATSign
{
  bool commit_valid_ = false;
  uint8_t rd_ = 0;
  uint8_t rob_id_ = 0;

  auto operator<=>(const ROBToRATSign &) const = default;
};

struct ROBToRegisterSign
{
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;

  auto operator<=>(const ROBToRegisterSign &) const = default;
};

struct MemToFetchSign
{
  bool inst_valid_ = false;
  uint32_t inst_ = 0;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;

  auto operator<=>(const MemToFetchSign &) const = default;
};
#endif
