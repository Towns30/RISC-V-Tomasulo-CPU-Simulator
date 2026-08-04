#include "ALU.hpp"

ALU::ALU(const RSToALUSign &rs_input_sign, ALUToCDBSign &cdb_output_sign,
         const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
             &flush_input_sign)
    : rs_input_sign_(rs_input_sign), cdb_output_sign_(cdb_output_sign),
      flush_input_sign_(flush_input_sign)
{
}

void ALU::UpdateCurrent() { current_state_ = next_state_; }

void ALU::Execute() // 利用current发送output
{
  cdb_output_sign_ = {};
  uint32_t result;
  uint32_t real_pc;
  if (current_state_.flush_input_.need_flush_)
  {
    return;
  }
  auto input = current_state_.rs_input_;
  switch (input.op_)
  {
  case Op::Add:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ + input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Sub:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ - input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::And:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ & input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Or:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ | input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Xor:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ ^ input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Sll:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ << (input.Vk_ & 0b11111u);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Srl:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ >> (input.Vk_ & 0b11111u);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Sra:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    int32_t signed_value = int32_t(input.Vj_);
    uint32_t offset = input.Vk_ & 0b11111u;
    int32_t shifted_value = signed_value >> offset;
    result = uint32_t(shifted_value);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Slt:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = int32_t(input.Vj_) < int32_t(input.Vk_);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Sltu:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ < input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Addi:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ + input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Andi:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ & input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Ori:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ | input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Xori:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ ^ input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Slli:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ << (input.Vk_ & 0b11111u);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Srli:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ >> (input.Vk_ & 0b11111u);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Srai:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    int32_t signed_value = int32_t(input.Vj_);
    uint32_t offset = input.Vk_ & 0b11111u;
    int32_t shifted_value = signed_value >> offset;
    result = uint32_t(shifted_value);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Slti:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = int32_t(input.Vj_) < int32_t(input.Vk_);
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Sltiu:
  {
    if (!input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    result = input.Vj_ < input.Vk_;
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = result;
    break;
  }

  case Op::Beq:
  {
    if (!input.pc_valid_ || !input.offset_valid_ || !input.Vj_valid_ ||
        !input.Vk_valid_)
    {
      return;
    }
    if (input.Vj_ == input.Vk_)
    {
      real_pc = input.pc_ + input.offset_;
    }
    else
    {
      real_pc = input.pc_ + 4;
    }
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = real_pc;
    break;
  }

  case Op::Bge:
  {
    if (!input.pc_valid_ || !input.offset_valid_ || !input.Vj_valid_ ||
        !input.Vk_valid_)
    {
      return;
    }
    if (int32_t(input.Vj_) >= int32_t(input.Vk_))
    {
      real_pc = input.pc_ + input.offset_;
    }
    else
    {
      real_pc = input.pc_ + 4;
    }
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = real_pc;
    break;
  }

  case Op::Bgeu:
  {
    if (!input.pc_valid_ || !input.offset_valid_ || !input.Vj_valid_ ||
        !input.Vk_valid_)
    {
      return;
    }
    if (input.Vj_ >= input.Vk_)
    {
      real_pc = input.pc_ + input.offset_;
    }
    else
    {
      real_pc = input.pc_ + 4;
    }
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = real_pc;
    break;
  }

  case Op::Blt:
  {
    if (!input.pc_valid_ || !input.offset_valid_ || !input.Vj_valid_ ||
        !input.Vk_valid_)
    {
      return;
    }
    if (int32_t(input.Vj_) < int32_t(input.Vk_))
    {
      real_pc = input.pc_ + input.offset_;
    }
    else
    {
      real_pc = input.pc_ + 4;
    }
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = real_pc;
    break;
  }

  case Op::Bltu:
  {
    if (!input.pc_valid_ || !input.offset_valid_ || !input.Vj_valid_ ||
        !input.Vk_valid_)
    {
      return;
    }
    if (input.Vj_ < input.Vk_)
    {
      real_pc = input.pc_ + input.offset_;
    }
    else
    {
      real_pc = input.pc_ + 4;
    }
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = real_pc;
    break;
  }

  case Op::Bne:
  {
    if (!input.pc_valid_ || !input.offset_valid_ || !input.Vj_valid_ ||
        !input.Vk_valid_)
    {
      return;
    }
    if (input.Vj_ != input.Vk_)
    {
      real_pc = input.pc_ + input.offset_;
    }
    else
    {
      real_pc = input.pc_ + 4;
    }
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = real_pc;
    break;
  }

  case Op::Jal:
  {
    if (!input.pc_valid_ || !input.offset_valid_)
    {
      return;
    }
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = input.pc_ + 4;
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = input.pc_ + input.offset_;
    break;
  }

  case Op::Jalr:
  {
    if (!input.pc_valid_ || !input.Vj_valid_ || !input.Vk_valid_)
    {
      return;
    }
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = input.pc_ + 4;
    cdb_output_sign_.real_pc_valid_ = true;
    cdb_output_sign_.real_pc_ = (input.Vj_ + input.Vk_) & ~1u;
    break;
  }

  case Op::Auipc:
  {
    if (!input.pc_valid_ || !input.Vk_valid_)
    {
      return;
    }
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = input.pc_ + input.Vk_;
    break;
  }

  case Op::Lui:
  {
    if (!input.Vk_valid_)
    {
      return;
    }
    cdb_output_sign_.value_valid_ = true;
    cdb_output_sign_.value_ = input.Vk_;
    break;
  }

  default:
    return;
  }

  if (input.rob_id_valid_)
  {
    cdb_output_sign_.rob_id_valid_ = true;
    cdb_output_sign_.rob_id_ = input.rob_id_;
  }
}

void ALU::UpdateNext() // 利用本周期input计算 next_state_
{
  next_state_.flush_input_ = flush_input_sign_;
  if (flush_input_sign_.need_flush_)
  {
    next_state_.rs_input_ = {};
    return;
  }
  next_state_.rs_input_ = rs_input_sign_;
}
