#ifndef ISSUE_HPP
#define ISSUE_HPP

#include "Sign.hpp"

class RAT;
class ROB;
class Register;

struct IssueState
{
  DecoderToIssueSign decoder_input_{};
  ALUToCDBSign alu_cdb_input_{};
  LSQToCDBSign lsq_cdb_input_{};
  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign flush_input_{};

  auto operator<=>(const IssueState &) const = default;
};

class Issue
{
public:
  Issue(const DecoderToIssueSign &decoder_input_sign,
        IssueToRATSign &rat_output_sign, IssueToLSQSign &lsq_output_sign,
        IssueToRSSign &rs_output_sign, IssueToROBSign &rob_output_sign,
        const ALUToCDBSign &alu_cdb_input_sign,
        const LSQToCDBSign &lsq_cdb_input_sign,
        const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
            &flush_input_sign);

  void SetReady(bool ready);
  void UpdateCurrent();
  void Execute(Register &registers, RAT &rat, ROB &rob);
  void UpdateNext();

private:
  const DecoderToIssueSign &decoder_input_sign_;
  IssueToRATSign &rat_output_sign_;
  IssueToLSQSign &lsq_output_sign_;
  IssueToRSSign &rs_output_sign_;
  IssueToROBSign &rob_output_sign_;
  const ALUToCDBSign &alu_cdb_input_sign_;
  const LSQToCDBSign &lsq_cdb_input_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  IssueState current_state_{};
  IssueState next_state_{};
  bool ready_ = true;
};

#endif
