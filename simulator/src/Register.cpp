#include "Register.hpp"

Register::Register(const ROBToRegisterSign &rob_input_sign)
    : rob_input_sign_(rob_input_sign)
{
}

uint32_t Register::Read(uint8_t register_id)
{
  if (register_id == 0)
  {
    return 0;
  }
  return current_state_.registers_[register_id];
}

void Register::UpdateCurrent() 
{
  current_state_ = next_state_;
}

void Register::Execute() // 利用current发送output
{
}

void Register::UpdateNext() // 利用 current_state_ 和本周期input计算 next_state_
{
  next_state_ = current_state_;
  if (rob_input_sign_.rd_valid_ && rob_input_sign_.value_valid_ && rob_input_sign_.rd_ != 0)
  {
    next_state_.registers_[rob_input_sign_.rd_] = rob_input_sign_.value_;
  }
  next_state_.registers_[0] = 0;
}
