#ifndef ALU_HPP
#define ALU_HPP

#include "Sign.hpp"

struct ALUState
{
  RSToALUSign rs_input_;
  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign flush_input_;

  auto operator<=>(const ALUState &) const = default;
};

class ALU
{
public:
  ALU(const RSToALUSign &rs_input_sign, ALUToCDBSign &cdb_output_sign,
      const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
          &flush_input_sign);

  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const RSToALUSign &rs_input_sign_;
  ALUToCDBSign &cdb_output_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  ALUState current_state_;
  ALUState next_state_;
};

#endif
