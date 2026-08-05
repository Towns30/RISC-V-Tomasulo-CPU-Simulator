#include "CPU.hpp"

CPU::CPU()
    : memory_(fetch_to_memory_sign_, memory_to_fetch_sign_, lsq_to_memory_sign_,
              memory_to_lsq_sign_, memory_to_rob_sign_, rob_flush_sign_),
      fetch_(fetch_to_memory_sign_, memory_to_fetch_sign_,
             fetch_to_decoder_sign_, rob_to_fetch_flush_sign_),
      decoder_(fetch_to_decoder_sign_, decoder_to_issue_sign_, rob_flush_sign_),
      register_(rob_to_register_sign_),
      rat_(issue_to_rat_sign_, rob_to_rat_sign_, rob_flush_sign_),
      rob_(issue_to_rob_sign_, lsq_to_rob_sign_, alu_to_cdb_sign_,
           lsq_to_cdb_sign_, memory_to_rob_sign_, rob_flush_sign_,
           rob_to_fetch_flush_sign_, rob_to_lsq_sign_, rob_to_rat_sign_,
           rob_to_register_sign_),
      issue_(decoder_to_issue_sign_, issue_to_rat_sign_, issue_to_lsq_sign_,
             issue_to_rs_sign_, issue_to_rob_sign_, alu_to_cdb_sign_,
             lsq_to_cdb_sign_, rob_flush_sign_, register_, rat_, rob_),
      rs_(issue_to_rs_sign_, rs_to_alu_sign_, alu_to_cdb_sign_,
          lsq_to_cdb_sign_, rob_flush_sign_),
      alu_(rs_to_alu_sign_, alu_to_cdb_sign_, rob_flush_sign_),
      lsq_(issue_to_lsq_sign_, lsq_to_memory_sign_, lsq_to_cdb_sign_,
           lsq_to_rob_sign_, alu_to_cdb_sign_, memory_to_lsq_sign_,
           rob_to_lsq_sign_, rob_flush_sign_)
{
}

void CPU::Cycle()
{
  UpdateCurrentAll();
  SetReadyAll();
  ClearSigns();
  ExecuteAll();
  UpdateNextAll();
}

void CPU::UpdateCurrentAll()
{
  memory_.UpdateCurrent();
  fetch_.UpdateCurrent();
  decoder_.UpdateCurrent();
  register_.UpdateCurrent();
  rat_.UpdateCurrent();
  rob_.UpdateCurrent();
  issue_.UpdateCurrent();
  rs_.UpdateCurrent();
  alu_.UpdateCurrent();
  lsq_.UpdateCurrent();
}

void CPU::SetReadyAll()
{
  bool ready = false;
  if (rs_.Ready() && lsq_.Ready() && rob_.Ready())
  {
    ready = true;
  }
  issue_.SetReady(ready);
  decoder_.SetReady(ready);
  fetch_.SetReady(ready);
  memory_.SetFetchReady(ready);
}

void CPU::ClearSigns()
{
  fetch_to_memory_sign_ = {};
  memory_to_fetch_sign_ = {};
  fetch_to_decoder_sign_ = {};
  decoder_to_issue_sign_ = {};

  issue_to_rat_sign_ = {};
  issue_to_lsq_sign_ = {};
  issue_to_rs_sign_ = {};
  issue_to_rob_sign_ = {};

  rs_to_alu_sign_ = {};
  alu_to_cdb_sign_ = {};
  lsq_to_cdb_sign_ = {};

  lsq_to_memory_sign_ = {};
  memory_to_lsq_sign_ = {};
  lsq_to_rob_sign_ = {};
  rob_to_lsq_sign_ = {};
  memory_to_rob_sign_ = {};

  rob_flush_sign_ = {};
  rob_to_fetch_flush_sign_ = {};
  rob_to_rat_sign_ = {};
  rob_to_register_sign_ = {};
}

void CPU::ExecuteAll()
{
  rob_.Execute();
  rs_.Execute();
  alu_.Execute();
  lsq_.Execute();
  memory_.Execute();
  register_.Execute();
  rat_.Execute();
  fetch_.Execute();
  decoder_.Execute();
  issue_.Execute();
}

void CPU::UpdateNextAll()
{
  memory_.UpdateNext();
  fetch_.UpdateNext();
  decoder_.UpdateNext();
  register_.UpdateNext();
  rat_.UpdateNext();
  rob_.UpdateNext();
  issue_.UpdateNext();
  rs_.UpdateNext();
  alu_.UpdateNext();
  lsq_.UpdateNext();
}
