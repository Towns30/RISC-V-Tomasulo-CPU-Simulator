#ifndef ROB_HPP
#define ROB_HPP

#include "Sign.hpp"

enum class WriteMemState
{
  NoWrite,
  NotDone
};

struct ROBInfo
{
  ROBType rob_type = ROBType::WriteReg;
  bool rd_valid_ = false;
  uint8_t rd_ = 0;
  bool rd_value_valid_ = false;
  uint32_t rd_value_ = 0;
  bool addr_valid_ = false;
  uint32_t addr_ = 0;
  bool addr_value_valid_ = false;
  uint32_t addr_value_ = 0;
  bool write_mem_state_valid_ = false;
  WriteMemState write_mem_state_ = WriteMemState::NoWrite;
  bool predict_pc_valid_ = false;
  uint32_t predict_pc_ = 0;
  bool real_pc_valid_ = false;
  uint32_t real_pc_ = 0;
  bool ready_ = false;

  auto operator<=>(const ROBInfo &) const = default;
};

struct ROBState
{
  ROBInfo rob_infos_[16]{};
  uint8_t head_ = 0;
  uint8_t tail_ = 0;

  auto operator<=>(const ROBState &) const = default;
};

struct ROBQueryResult
{
  bool valid_ = false;
  uint32_t value_ = 0;

  auto operator<=>(const ROBQueryResult &) const = default;
};

struct ROBQueryIDResult
{
  bool valid_ = false;
  uint32_t rob_id_ = 0;

  auto operator<=>(const ROBQueryIDResult &) const = default;
};

class ROB
{
public:
  ROB(const IssueToROBSign &issue_input_sign,
      const LSQToROBSign &lsq_input_sign,
      const ALUToCDBSign &alu_cdb_input_sign,
      const LSQToCDBSign &lsq_cdb_input_sign,
      const MemToROBSign &memory_input_sign,
      ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign &flush_output_sign,
      ROBToFetchFlushSign &fetch_flush_output_sign,
      ROBToLSQSign &lsq_output_sign, ROBToRATSign &rat_output_sign,
      ROBToRegisterSign &register_output_sign,
      ROBToCPUHaltSign &halt_output_sign);

  bool Ready();
  ROBQueryIDResult NextId();
  ROBQueryResult Query(uint8_t rob_id);

  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const IssueToROBSign &issue_input_sign_;
  const LSQToROBSign &lsq_input_sign_;
  const ALUToCDBSign &alu_cdb_input_sign_;
  const LSQToCDBSign &lsq_cdb_input_sign_;
  const MemToROBSign &memory_input_sign_;
  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign &flush_output_sign_;
  ROBToFetchFlushSign &fetch_flush_output_sign_;
  ROBToLSQSign &lsq_output_sign_;
  ROBToRATSign &rat_output_sign_;
  ROBToRegisterSign &register_output_sign_;
  ROBToCPUHaltSign &halt_output_sign_;
  ROBState current_state_;
  ROBState next_state_;
};

#endif
