#ifndef CPU_HPP
#define CPU_HPP

#include "ALU.hpp"
#include "Decoder.hpp"
#include "Fetch.hpp"
#include "Issue.hpp"
#include "LSQ.hpp"
#include "Memory.hpp"
#include "RAT.hpp"
#include "ROB.hpp"
#include "RS.hpp"
#include "Register.hpp"
#include "Sign.hpp"

#include <iosfwd>

class CPU
{
public:
  CPU();

  void LoadProgram(std::istream &input);
  void Cycle();
  bool Halted() const;
  uint32_t Result() const;

  CPU(const CPU &) = delete;
  CPU &operator=(const CPU &) = delete;
  CPU(CPU &&) = delete;
  CPU &operator=(CPU &&) = delete;

private:
  void SetReadyAll();
  void UpdateCurrentAll();
  void ClearSigns();
  void ExecuteAll();
  void UpdateNextAll();

  FetchToMemSign fetch_to_memory_sign_{};
  MemToFetchSign memory_to_fetch_sign_{};
  FetchToDecoderSign fetch_to_decoder_sign_{};
  DecoderToIssueSign decoder_to_issue_sign_{};

  IssueToRATSign issue_to_rat_sign_{};
  IssueToLSQSign issue_to_lsq_sign_{};
  IssueToRSSign issue_to_rs_sign_{};
  IssueToROBSign issue_to_rob_sign_{};

  RSToALUSign rs_to_alu_sign_{};
  ALUToCDBSign alu_to_cdb_sign_{};
  LSQToCDBSign lsq_to_cdb_sign_{};

  LSQToMemSign lsq_to_memory_sign_{};
  MemToLSQSign memory_to_lsq_sign_{};
  LSQToROBSign lsq_to_rob_sign_{};
  ROBToLSQSign rob_to_lsq_sign_{};
  MemToROBSign memory_to_rob_sign_{};

  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign rob_flush_sign_{};
  ROBToFetchFlushSign rob_to_fetch_flush_sign_{};
  ROBToRATSign rob_to_rat_sign_{};
  ROBToRegisterSign rob_to_register_sign_{};
  ROBToCPUHaltSign rob_to_cpu_halt_sign_{};

  bool halted_ = false;
  uint32_t result_ = 0;

  Memory memory_;
  Fetch fetch_;
  Decoder decoder_;
  Register register_;
  RAT rat_;
  ROB rob_;
  Issue issue_;
  RS rs_;
  ALU alu_;
  LSQ lsq_;
};

#endif
