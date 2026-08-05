#include "Memory.hpp"

Memory::Memory(const FetchToMemSign &fetch_input_sign,
               MemToFetchSign &fetch_output_sign,
               const LSQToMemSign &lsq_input_sign,
               MemToLSQSign &lsq_output_sign, MemToROBSign &rob_output_sign,
               const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
                   &flush_input_sign)
    : fetch_input_sign_(fetch_input_sign),
      fetch_output_sign_(fetch_output_sign), lsq_input_sign_(lsq_input_sign),
      lsq_output_sign_(lsq_output_sign), rob_output_sign_(rob_output_sign),
      flush_input_sign_(flush_input_sign)
{
}

void Memory::SetFetchReady(bool fetch_ready) { fetch_ready_ = fetch_ready; }

void Memory::UpdateCurrent() { current_state_ = next_state_; }

void Memory::Execute() // 利用current发送output，以及修改next
{
  next_state_ = current_state_;
  fetch_output_sign_ = {};
  if (current_state_.flush_input_.need_flush_)
  {
    return;
  }
  if (current_state_.writing_reading_state_.is_reading_)
  {
    switch (current_state_.writing_reading_state_.reading_state_)
    {
    case 0:
      next_state_.writing_reading_state_.reading_state_ = 1;
      break;
    case 1:
      next_state_.writing_reading_state_.reading_state_ = 2;
      break;
    case 2:
    {
      uint32_t reading_addr =
          current_state_.writing_reading_state_.reading_addr_;
      uint32_t reading_bytes =
          current_state_.writing_reading_state_.reading_bytes_;

      next_state_.writing_reading_state_.is_reading_ = false;
      next_state_.writing_reading_state_.reading_state_ = 0;

      // 小端序
      uint32_t read_value = 0;
      for (uint32_t i = 0; i < reading_bytes; i++)
      {
        uint32_t byte_value = uint32_t(memory_[reading_addr + i]);
        read_value |= byte_value << (i * 8);
      }

      // 有符号扩展
      if (current_state_.writing_reading_state_.reading_type_ == 1 &&
          reading_bytes < 4)
      {
        uint32_t value_bits = reading_bytes * 8;
        uint32_t sign_bit = 1u << (value_bits - 1);
        if ((read_value & sign_bit) != 0) // 最高位为1
        {
          uint32_t extension_mask = 0xffffffffu << value_bits;
          read_value |= extension_mask;
        }
      }

      lsq_output_sign_.rob_id_valid_ = true;
      lsq_output_sign_.rob_id_ =
          current_state_.writing_reading_state_.reading_rob_id_;
      lsq_output_sign_.value_valid_ = true;
      lsq_output_sign_.value_ = read_value;
      break;
    }
    }
  }
  if (current_state_.writing_reading_state_.is_writing_)
  {
    switch (current_state_.writing_reading_state_.writing_state_)
    {
    case 0:
      next_state_.writing_reading_state_.writing_state_ = 1;
      break;
    case 1:
      next_state_.writing_reading_state_.writing_state_ = 2;
      break;
    case 2:
    {
      uint32_t writing_addr =
          current_state_.writing_reading_state_.writing_addr_;
      uint32_t writing_value =
          current_state_.writing_reading_state_.writing_value_;
      uint32_t writing_bytes =
          current_state_.writing_reading_state_.writing_bytes_;

      next_state_.writing_reading_state_.is_writing_ = false;
      next_state_.writing_reading_state_.writing_state_ = 0;

      // 小端序
      for (uint32_t i = 0; i < writing_bytes; i++)
      {
        uint32_t shifted_value = writing_value >> (i * 8);
        memory_[writing_addr + i] = uint8_t(shifted_value & 0xffu);
      }

      lsq_output_sign_.is_written_ = true;
      lsq_output_sign_.rob_id_valid_ = true;
      lsq_output_sign_.rob_id_ =
          current_state_.writing_reading_state_.writing_rob_id_;
      rob_output_sign_.is_written_ = true;
      rob_output_sign_.rob_id_valid_ = true;
      rob_output_sign_.rob_id_ =
          current_state_.writing_reading_state_.writing_rob_id_;
      break;
    }
    }
  }
  if (fetch_ready_ && current_state_.fetch_input_.pc_valid_) // 只有fetch不阻塞时，此时fetch也会发来有效pc，这时才能给fetch发消息
  {
    uint32_t pc = current_state_.fetch_input_.pc_;
    uint32_t byte0 = uint32_t(memory_[pc]);
    uint32_t byte1 = uint32_t(memory_[pc + 1]);
    uint32_t byte2 = uint32_t(memory_[pc + 2]);
    uint32_t byte3 = uint32_t(memory_[pc + 3]);
    uint32_t inst = byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
    fetch_output_sign_.inst_valid_ = true;
    fetch_output_sign_.inst_ = inst;
    fetch_output_sign_.pc_valid_ = true;
    fetch_output_sign_.pc_ = pc;
  }
}

void Memory::UpdateNext() // 利用本周期input以及current计算 next_state_
{
  next_state_.fetch_input_ = fetch_input_sign_;
  next_state_.flush_input_ = flush_input_sign_;
  if (flush_input_sign_.need_flush_)
  {
    next_state_.fetch_input_ = {};
    next_state_.writing_reading_state_ = WritingReadingState{};
    return;
  }
  if (!fetch_ready_) // fetch阻塞，下周期需缓存本周期的fetch_input_
  {
    next_state_.fetch_input_ = current_state_.fetch_input_;
  }
  if (lsq_input_sign_.rob_id_valid_ && lsq_input_sign_.addr_valid_)
  {
    if (lsq_input_sign_.value_valid_) // 写内存
    {
      next_state_.writing_reading_state_.is_writing_ = true;
      next_state_.writing_reading_state_.writing_state_ = 0;
      next_state_.writing_reading_state_.writing_addr_ = lsq_input_sign_.addr_;
      next_state_.writing_reading_state_.writing_bytes_ = lsq_input_sign_.bytes_;
      next_state_.writing_reading_state_.writing_rob_id_ = lsq_input_sign_.rob_id_;
      next_state_.writing_reading_state_.writing_value_ = lsq_input_sign_.value_;
    }
    else // 读内存
    {
      next_state_.writing_reading_state_.is_reading_ = true;
      next_state_.writing_reading_state_.reading_state_ = 0;
      next_state_.writing_reading_state_.reading_addr_ = lsq_input_sign_.addr_;
      next_state_.writing_reading_state_.reading_rob_id_ = lsq_input_sign_.rob_id_;
      next_state_.writing_reading_state_.reading_bytes_ = lsq_input_sign_.bytes_;
      next_state_.writing_reading_state_.reading_type_ = lsq_input_sign_.is_signed_; 
    }
  }
}
