#ifndef DECODER_HPP
#define DECODER_HPP

#include "Issue.hpp"
#include "Sign.hpp"

struct DecoderState
{
  FetchToDecoderSign fetch_input_;
  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign flush_input_;
  auto operator<=>(const DecoderState &) const = default;
};

class Decoder
{
public:
  Decoder(const FetchToDecoderSign &fetch_input_sign,
          DecoderToIssueSign &issue_output_sign,
          const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
              &flush_input_sign);

  void SetReady(bool ready);
  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const FetchToDecoderSign &fetch_input_sign_;
  DecoderToIssueSign &issue_output_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  DecoderState current_state_;
  DecoderState next_state_;
  bool ready_ = true;
};

#endif
