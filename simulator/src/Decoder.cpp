#include "Decoder.hpp"

Decoder::Decoder(const FetchToDecoderSign &fetch_input_sign,
                 DecoderToIssueSign &issue_output_sign,
                 const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
                     &flush_input_sign)
    : fetch_input_sign_(fetch_input_sign),
      issue_output_sign_(issue_output_sign), flush_input_sign_(flush_input_sign)
{
}

void Decoder::SetReady(bool ready) { ready_ = ready; }

void Decoder::UpdateCurrent() { current_state_ = next_state_; }

void Decoder::Execute() // 利用current发送output
{
  issue_output_sign_ = {};
  if (current_state_.flush_input_.need_flush_)
  {
    return;
  }
  if (!ready_)
  {
    return;
  }
  if (!current_state_.fetch_input_.inst_valid_ ||
      !current_state_.fetch_input_.pc_valid_)
  {
    return;
  }
  const uint32_t inst = current_state_.fetch_input_.inst_;
  issue_output_sign_.raw_instruction_valid_ = true;
  issue_output_sign_.raw_instruction_ = inst;

  issue_output_sign_.pc_valid_ = true;
  issue_output_sign_.pc_ = current_state_.fetch_input_.pc_;

  issue_output_sign_.predicted_next_pc_valid_ =
      current_state_.fetch_input_.predicted_next_pc_valid_;
  issue_output_sign_.predicted_next_pc_ =
      current_state_.fetch_input_.predicted_next_pc_;

  if (inst == 0x0ff00513u)
  {
    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = Op::Halt;
    return;
  }
  
  const uint32_t opcode = inst & 0x7f;
  const uint8_t rd = (inst >> 7) & 0b11111u;
  const uint32_t funct3 = (inst >> 12) & 0b111u;
  const uint8_t rs1 = (inst >> 15) & 0b11111u;
  const uint8_t rs2 = (inst >> 20) & 0b11111u;
  const uint32_t funct7 = (inst >> 25) & 0b1111111u;
  switch (opcode)
  {
  case 0b0110011u: // R
  {
    Op decoded_op = Op::Invalid;
    switch (funct3)
    {
    case 0b000u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Add;
      }
      else if (funct7 == 0b0100000u)
      {
        decoded_op = Op::Sub;
      }
      break;

    case 0b111u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::And;
      }
      break;

    case 0b110u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Or;
      }
      break;

    case 0b100u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Xor;
      }
      break;

    case 0b001u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Sll;
      }
      break;

    case 0b101u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Srl;
      }
      else if (funct7 == 0b0100000u)
      {
        decoded_op = Op::Sra;
      }
      break;

    case 0b010u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Slt;
      }
      break;

    case 0b011u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Sltu;
      }
      break;

    default:
      break;
    }

    if (decoded_op == Op::Invalid)
    {
      return;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = decoded_op;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.rs1_valid_ = true;
    issue_output_sign_.rs1_ = rs1;

    issue_output_sign_.rs2_valid_ = true;
    issue_output_sign_.rs2_ = rs2;
    break;
  }
  case uint32_t(0b0010011): // I
  {
    Op decoded_op = Op::Invalid;
    uint32_t decoded_imm = inst >> 20;
    if ((decoded_imm & 0b100000000000u) != 0) // 若最高位为1，则高位补1
    {
      decoded_imm |= 0xfffff000u;
    }
    switch (funct3)
    {
    case 0b000u:
      decoded_op = Op::Addi;
      break;

    case 0b111u:
      decoded_op = Op::Andi;
      break;

    case 0b110u:
      decoded_op = Op::Ori;
      break;

    case 0b100u:
      decoded_op = Op::Xori;
      break;

    case 0b001u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Slli;
        decoded_imm = rs2;
      }
      break;

    case 0b101u:
      if (funct7 == 0b0000000u)
      {
        decoded_op = Op::Srli;
        decoded_imm = rs2;
      }
      else if (funct7 == 0b0100000u)
      {
        decoded_op = Op::Srai;
        decoded_imm = rs2;
      }
      break;

    case 0b010u:
      decoded_op = Op::Slti;
      break;

    case 0b011u:
      decoded_op = Op::Sltiu;
      break;

    default:
      break;
    }

    if (decoded_op == Op::Invalid)
    {
      return;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = decoded_op;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.rs1_valid_ = true;
    issue_output_sign_.rs1_ = rs1;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = decoded_imm;

    break;
  }
  case uint32_t(0b0000011): // L
  {
    Op decoded_op = Op::Invalid;
    switch (funct3)
    {
    case 0b000u:
      decoded_op = Op::Lb;
      break;

    case 0b100u:
      decoded_op = Op::Lbu;
      break;

    case 0b001u:
      decoded_op = Op::Lh;
      break;

    case 0b101u:
      decoded_op = Op::Lhu;
      break;

    case 0b010u:
      decoded_op = Op::Lw;
      break;

    default:
      break;
    }
    if (decoded_op == Op::Invalid)
    {
      return;
    }
    uint32_t decoded_imm = inst >> 20;
    if ((decoded_imm & 0b100000000000u) != 0)
    {
      decoded_imm |= 0xfffff000u;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = decoded_op;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.rs1_valid_ = true;
    issue_output_sign_.rs1_ = rs1;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = decoded_imm;

    break;
  }
  case uint32_t(0b0100011): // S
  {
    Op decoded_op = Op::Invalid;
    switch (funct3)
    {
    case 0b000u:
      decoded_op = Op::Sb;
      break;

    case 0b001u:
      decoded_op = Op::Sh;
      break;

    case 0b010u:
      decoded_op = Op::Sw;
      break;

    default:
      break;
    }
    if (decoded_op == Op::Invalid)
    {
      return;
    }
    uint32_t decoded_imm = ((inst >> 25) << 5) | ((inst >> 7) & 0b11111u);
    if ((decoded_imm & 0b100000000000u) != 0)
    {
      decoded_imm |= 0xfffff000u;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = decoded_op;

    issue_output_sign_.rs1_valid_ = true;
    issue_output_sign_.rs1_ = rs1;

    issue_output_sign_.rs2_valid_ = true;
    issue_output_sign_.rs2_ = rs2;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = decoded_imm;

    break;
  }
  case uint32_t(0b1100011): // B
  {
    Op decoded_op = Op::Invalid;
    switch (funct3)
    {
    case 0b000u:
      decoded_op = Op::Beq;
      break;

    case 0b101u:
      decoded_op = Op::Bge;
      break;

    case 0b111u:
      decoded_op = Op::Bgeu;
      break;

    case 0b100u:
      decoded_op = Op::Blt;
      break;

    case 0b110u:
      decoded_op = Op::Bltu;
      break;

    case 0b001u:
      decoded_op = Op::Bne;
      break;

    default:
      break;
    }
    if (decoded_op == Op::Invalid)
    {
      return;
    }
    uint32_t decoded_imm =
        ((inst >> 31) & 0b1u) << 12 | ((inst >> 7) & 0b1u) << 11 |
        ((inst >> 25) & 0b111111u) << 5 | ((inst >> 8) & 0b1111u) << 1;
    if ((decoded_imm & 0b1000000000000u) != 0)
    {
      decoded_imm |= 0xffffe000u;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = decoded_op;

    issue_output_sign_.rs1_valid_ = true;
    issue_output_sign_.rs1_ = rs1;

    issue_output_sign_.rs2_valid_ = true;
    issue_output_sign_.rs2_ = rs2;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = decoded_imm;

    break;
  }
  case uint32_t(0b1101111): // J
  {
    uint32_t decoded_imm =
        ((inst >> 31) & 0b1u) << 20 | ((inst >> 12) & 0b11111111u) << 12 |
        ((inst >> 20) & 0b1u) << 11 | ((inst >> 21) & 0b1111111111u) << 1;
    if ((decoded_imm & 0b100000000000000000000u) != 0)
    {
      decoded_imm |= 0xffe00000u;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = Op::Jal;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = decoded_imm;

    break;
  }
  case uint32_t(0b1100111): // JALR(I)
  {
    if (funct3 != 0b000u)
    {
      return;
    }
    uint32_t decoded_imm = inst >> 20;
    if ((decoded_imm & 0b100000000000u) != 0)
    {
      decoded_imm |= 0xfffff000u;
    }

    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = Op::Jalr;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.rs1_valid_ = true;
    issue_output_sign_.rs1_ = rs1;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = decoded_imm;

    break;
  }
  case uint32_t(0b0010111): // AUIPC
  {
    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = Op::Auipc;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = inst & 0xfffff000u;

    break;
  }
  case uint32_t(0b0110111): // LUI
  {
    issue_output_sign_.op_valid_ = true;
    issue_output_sign_.op_ = Op::Lui;

    issue_output_sign_.rd_valid_ = (rd != 0);
    issue_output_sign_.rd_ = rd;

    issue_output_sign_.imm_valid_ = true;
    issue_output_sign_.imm_ = inst & 0xfffff000u;

    break;
  }
  }
}

void Decoder::UpdateNext() // 利用本周期input计算 next_state_
{
  next_state_.flush_input_ = flush_input_sign_;
  if (flush_input_sign_
          .need_flush_) // 如果需要flush，就直接让下一次指令输入为空
  {
    next_state_.fetch_input_ = {};
    return;
  }
  if (!ready_ &&
      !current_state_.flush_input_
           .need_flush_) // 此周期未解码current且不需要flush，则将next置成current，防止指令丢失
  {
    next_state_ = current_state_;
    return;
  }
  next_state_ = DecoderState{
      .fetch_input_ = fetch_input_sign_,
      .flush_input_ = flush_input_sign_,
  };
}
