#ifndef SIGN_HPP
#define SIGN_HPP
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
  Sll,
  Slt,
  Sltu,
  Xor,
  Srl,
  Sra,
  Or,
  And,
};

struct FetchToMemSign
{
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
};

struct FetchToDecoderSign
{
  bool inst_valid_ = false;
  uint32_t inst_ = 0;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
  bool predicted_next_pc_valid_ = false;
  uint32_t predicted_next_pc_ = 0;
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
};

struct IssueToRATSign
{
  bool register_id_valid_ = false;
  uint8_t register_id_ = 0;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
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
};

struct IssueToROBSign
{
  Op op_ = Op::Invalid;
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool predicted_pc_valid_ = false;
  uint32_t predicted_pc_ = 0;
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
};

struct LSQToCDBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
};

struct LSQToROBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool addr_valid_ = false;
  uint32_t addr_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
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
};

struct ALUToCDBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;
};

struct CDBToRS_LSQ_ROB_IssueSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;
};

struct MemToLSQSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
  bool is_written_ = false;
};

struct MemToROBSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool is_written_ = false;
};

struct ROBToIssue_RAT_LSQ_RS_Decoder_CDB_MemoryFlushSign
{
  bool need_flush_ = false;
};

struct ROBToFetchFlushSign
{
  bool need_flush_ = false;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;
};

struct ROBToLSQSign
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
  bool write_enabled_ = false;
};

struct ROBToRATSign
{
  bool commit_valid_ = false;
  uint8_t rd_ = 0;
  uint8_t rob_id_ = 0;
};

struct ROBToRegisterSign
{
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool value_valid_ = false;
  uint32_t value_ = 0;
};

struct MemToFetchSign
{
  bool inst_valid_ = false;
  uint32_t inst_ = 0;
};
#endif
