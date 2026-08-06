#include "CPU.hpp"

#include <algorithm>
#include <array>
#include <ctime>

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
           rob_to_register_sign_, rob_to_cpu_halt_sign_),
      issue_(decoder_to_issue_sign_, issue_to_rat_sign_, issue_to_lsq_sign_,
             issue_to_rs_sign_, issue_to_rob_sign_, alu_to_cdb_sign_,
             lsq_to_cdb_sign_, rob_flush_sign_),
      rs_(issue_to_rs_sign_, rs_to_alu_sign_, alu_to_cdb_sign_,
          lsq_to_cdb_sign_, rob_flush_sign_),
      alu_(rs_to_alu_sign_, alu_to_cdb_sign_, rob_flush_sign_),
      lsq_(issue_to_lsq_sign_, lsq_to_memory_sign_, lsq_to_cdb_sign_,
           lsq_to_rob_sign_, alu_to_cdb_sign_, memory_to_lsq_sign_,
           rob_to_lsq_sign_, rob_flush_sign_)
{
  random_engine_.seed(static_cast<unsigned int>(std::time(nullptr)));
}

void CPU::LoadProgram(std::istream &input) { memory_.LoadProgram(input); }

void CPU::Cycle()
{
  if (halted_)
  {
    return;
  }
  UpdateCurrentAll();
  SetReadyAll();
  ClearSigns();
  ExecuteAll();
  if (halted_)
  {
    return;
  }
  UpdateNextAll();
}

bool CPU::Halted() const { return halted_; }

uint32_t CPU::Result() const { return result_; }

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
  rob_to_cpu_halt_sign_ = {};
}

void CPU::ExecuteAll()
{
  std::array<int, 10> execute_order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::shuffle(execute_order.begin(), execute_order.end(), random_engine_);

  for (int module : execute_order)
  {
    switch (module)
    {
    case 0:
    {
      rob_.Execute();
      break;
    }
    case 1:
    {
      rs_.Execute();
      break;
    }
    case 2:
    {
      alu_.Execute();
      break;
    }
    case 3:
    {
      lsq_.Execute();
      break;
    }
    case 4:
    {
      memory_.Execute();
      break;
    }
    case 5:
    {
      register_.Execute();
      break;
    }
    case 6:
    {
      rat_.Execute();
      break;
    }
    case 7:
    {
      fetch_.Execute();
      break;
    }
    case 8:
    {
      decoder_.Execute();
      break;
    }
    case 9:
    {
      issue_.Execute(register_, rat_, rob_);
      break;
    }
    }

    if (rob_to_cpu_halt_sign_.need_halt_)
    {
      halted_ = true;
      result_ = register_.Read(10) & 0xffu;
      return;
    }
  }
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
