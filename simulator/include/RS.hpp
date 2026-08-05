#ifndef RS_HPP
#define RS_HPP

#include "Sign.hpp"

struct RSInfo
{
  bool info_valid_ = false;
  uint8_t rob_id_ = 0;
  Op op_ = Op::Invalid;
  bool pc_valid_ = false;
  uint32_t pc_ = 0;
  bool offset_valid_ = false;
  uint32_t offset_ = 0;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool vj_valid_ = false;
  uint32_t vj_ = 0;
  bool vk_valid_ = false;
  uint32_t vk_ = 0;
  bool qj_valid_ = false;
  uint8_t qj_ = 0;
  bool qk_valid_ = false;
  uint8_t qk_ = 0;

  auto operator<=>(const RSInfo &) const = default;
};

struct RSState
{
  RSInfo rs_infos_[16];
  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign flush_input_;
};

class RS
{
public:
  RS(const IssueToRSSign &issue_input_sign, RSToALUSign &alu_output_sign,
     const ALUToCDBSign &alu_cdb_input_sign,
     const LSQToCDBSign &lsq_cdb_input_sign,
     const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
         &flush_input_sign);

  bool Ready();

  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const IssueToRSSign &issue_input_sign_;
  RSToALUSign &alu_output_sign_;
  const ALUToCDBSign &alu_cdb_input_sign_;
  const LSQToCDBSign &lsq_cdb_input_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  RSState current_state_;
  RSState next_state_;
};

#endif
