#include "Fetch.hpp"

Fetch::Fetch(FetchToMemSign &memory_output_sign,
             const MemToFetchSign &memory_input_sign,
             FetchToDecoderSign &decoder_output_sign,
             const ROBToFetchFlushSign &flush_input_sign)
    : memory_output_sign_(memory_output_sign),
      memory_input_sign_(memory_input_sign),
      decoder_output_sign_(decoder_output_sign),
      flush_input_sign_(flush_input_sign)
{
}

void Fetch::SetReady(bool ready) { ready_ = ready; }

void Fetch::UpdateCurrent() { current_state_ = next_state_; }

void Fetch::Execute() // 利用current发送output
{
  decoder_output_sign_ = {};
  memory_output_sign_ = {};
  if (current_state_.flush_input_
          .need_flush_) // 需要flush，此时需要更新此周期向Mem发送的pc
  {
    if (current_state_.flush_input_.real_pc_valid_)
    {
      memory_output_sign_.pc_valid_ = true;
      memory_output_sign_.pc_ = current_state_.flush_input_.real_pc_;
    }
    return;
  }
  // 若需要flush本身就不会发送，所以可以先判断flush
  if (!ready_) // 如果阻塞就不发，下一周期的pc还是当周期的
  {
    return;
  }
  // 用now_pc请求指令
  memory_output_sign_.pc_valid_ = true;
  memory_output_sign_.pc_ = current_state_.now_pc_;
  if (!current_state_.memory_input_.inst_valid_ ||
      !current_state_.memory_input_.pc_valid_)
  {
    return;
  }
  auto inst = current_state_.memory_input_.inst_;
  decoder_output_sign_.inst_valid_ = true;
  decoder_output_sign_.inst_ = inst;
  decoder_output_sign_.pc_valid_ = true;
  decoder_output_sign_.pc_ = current_state_.memory_input_.pc_;
  decoder_output_sign_.predicted_next_pc_valid_ = true;
  decoder_output_sign_.predicted_next_pc_ =
      current_state_.memory_input_.pc_ + 4;
}

void Fetch::UpdateNext() // 利用本周期input计算 next_state_
{
  next_state_.flush_input_ = flush_input_sign_;
  next_state_.memory_input_ = memory_input_sign_;
  if (current_state_.flush_input_
          .need_flush_) // 需要flush，此时需要更新此周期向Mem发送的pc
  {
    if (current_state_.flush_input_.real_pc_valid_)
    {
      next_state_.now_pc_ = current_state_.flush_input_.real_pc_ + 4;
    }
    next_state_.memory_input_ = {}; // 此周期内存返回的指令已经被flush了
    return;
  }
  // 若需要flush本身就不会发送，所以可以先判断flush
  if (!ready_) // 如果阻塞就不发，下一周期的pc还是当周期的
  {
    next_state_.now_pc_ = current_state_.now_pc_;
    if (current_state_.memory_input_.inst_valid_)
    {
      next_state_.memory_input_ = current_state_.memory_input_;
    }
    return;
  }
  next_state_.now_pc_ = current_state_.now_pc_ + 4;
}
