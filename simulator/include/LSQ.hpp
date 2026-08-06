#ifndef LSQ_HPP
#define LSQ_HPP

#include "Sign.hpp"

enum class LSQType
{
  Store,
  Load
};

struct LSQInfo
{
  bool info_valid_ = false;
  Op op_ = Op::Invalid;
  uint8_t state_ = 0; // 0，1，2，3, 4
  LSQType lsq_type_ = LSQType::Store;
  uint8_t rob_id_ = 0;
  bool vj_valid_ = false;
  uint32_t vj_ = 0;
  bool vk_valid_ = false;
  uint32_t vk_ = 0;
  bool qj_valid_ = false;
  uint8_t qj_ = 0;
  bool qk_valid_ = false;
  uint8_t qk_ = 0;
  bool imm_valid_ = false;
  uint32_t imm_ = 0;
  bool addr_valid_ = false;
  uint32_t addr_ = 0;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;

  auto operator<=>(const LSQInfo &) const = default;
};

struct LSQState
{
  LSQInfo lsq_infos_[16]{};
  uint8_t head_ = 0;
  uint8_t tail_ = 0;
  MemToLSQSign memory_input_;
};


class LSQ
{
public:
  LSQ(const IssueToLSQSign &issue_input_sign, LSQToMemSign &memory_output_sign,
      LSQToCDBSign &cdb_output_sign, LSQToROBSign &rob_output_sign,
      const ALUToCDBSign &alu_cdb_input_sign,
      const MemToLSQSign &memory_input_sign, const ROBToLSQSign &rob_input_sign,
      const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
          &flush_input_sign);

  bool Ready() const;

  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const IssueToLSQSign &issue_input_sign_;
  LSQToMemSign &memory_output_sign_;
  LSQToCDBSign &cdb_output_sign_;
  LSQToROBSign &rob_output_sign_;
  const ALUToCDBSign &alu_cdb_input_sign_;
  const MemToLSQSign &memory_input_sign_;
  const ROBToLSQSign &rob_input_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  LSQState current_state_;
  LSQState next_state_;
};

#endif
