#ifndef RAT_HPP
#define RAT_HPP

#include "Sign.hpp"

struct RobID
{
  bool is_empty_ = true;
  uint8_t rob_id_ = 0;

  auto operator<=>(const RobID &) const = default;
};

struct RATState
{
  RobID rat_map_[32];

  auto operator<=>(const RATState &) const = default;
};

struct RATQueryResult
{
  bool rob_id_valid_ = false;
  uint8_t rob_id_ = 0;
};

class RAT
{
public:
  RAT(const IssueToRATSign &issue_input_sign,
      const ROBToRATSign &rob_input_sign,
      const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
          &flush_input_sign);

  RATQueryResult Query(uint8_t register_id);

  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const IssueToRATSign &issue_input_sign_;
  const ROBToRATSign &rob_input_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  RATState current_state_;
  RATState next_state_;
};

#endif
