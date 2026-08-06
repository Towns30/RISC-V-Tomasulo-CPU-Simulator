#include "Issue.hpp"
#include "RAT.hpp"
#include "ROB.hpp"
#include "Register.hpp"

Issue::Issue(const DecoderToIssueSign &decoder_input_sign,
             IssueToRATSign &rat_output_sign, IssueToLSQSign &lsq_output_sign,
             IssueToRSSign &rs_output_sign, IssueToROBSign &rob_output_sign,
             const ALUToCDBSign &alu_cdb_input_sign,
             const LSQToCDBSign &lsq_cdb_input_sign,
             const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
                 &flush_input_sign)
    : decoder_input_sign_(decoder_input_sign),
      rat_output_sign_(rat_output_sign), lsq_output_sign_(lsq_output_sign),
      rs_output_sign_(rs_output_sign), rob_output_sign_(rob_output_sign),
      alu_cdb_input_sign_(alu_cdb_input_sign),
      lsq_cdb_input_sign_(lsq_cdb_input_sign),
      flush_input_sign_(flush_input_sign)
{
}

void Issue::SetReady(bool ready) { ready_ = ready; }

void Issue::UpdateCurrent() { current_state_ = next_state_; }

void Issue::Execute(Register &registers, RAT &rat,
                    ROB &rob) // 利用current发送output，以及修改next
{
  if (!ready_)
  {
    return;
  }
  if (current_state_.flush_input_.need_flush_)
  {
    return;
  }
  auto decoder_input = current_state_.decoder_input_;
  if (decoder_input.raw_instruction_valid_ && decoder_input.op_valid_)
  {
    uint8_t rob_id = rob.NextId().rob_id_; // 一定能取出
    Op now_op = decoder_input.op_;
    if (now_op == Op::Add || now_op == Op::Sub || now_op == Op::Xor ||
        now_op == Op::Or || now_op == Op::And || now_op == Op::Sll ||
        now_op == Op::Srl || now_op == Op::Sra || now_op == Op::Slt ||
        now_op == Op::Sltu)
    {
      if (decoder_input.rs1_valid_ &&
          decoder_input.rs2_valid_) // 不要求rd_valid 因为有可能是rd = 0
      {
        // 发给rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rs_output_sign_.rd_ = decoder_input.rd_;
        auto rs1_query = rat.Query(decoder_input.rs1_);
        auto rs2_query = rat.Query(decoder_input.rs2_);
        if (rs1_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(rs1_query.rob_id_).valid_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = rob.Query(rs1_query.rob_id_).value_;
          }
          else
          {
            rs_output_sign_.Qj_valid_ = true;
            rs_output_sign_.Qj_ = rs1_query.rob_id_;
          }
        }
        else
        {
          rs_output_sign_.Vj_valid_ = true;
          rs_output_sign_.Vj_ = registers.Read(decoder_input.rs1_);
        }
        if (rs2_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == rs2_query.rob_id_)
          {
            rs_output_sign_.Vk_valid_ = true;
            rs_output_sign_.Vk_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == rs2_query.rob_id_)
          {
            rs_output_sign_.Vk_valid_ = true;
            rs_output_sign_.Vk_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(rs2_query.rob_id_).valid_)
          {
            rs_output_sign_.Vk_valid_ = true;
            rs_output_sign_.Vk_ = rob.Query(rs2_query.rob_id_).value_;
          }
          else
          {
            rs_output_sign_.Qk_valid_ = true;
            rs_output_sign_.Qk_ = rs2_query.rob_id_;
          }
        }
        else
        {
          rs_output_sign_.Vk_valid_ = true;
          rs_output_sign_.Vk_ = registers.Read(decoder_input.rs2_);
        }
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::WriteReg;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = true;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
    else if (now_op == Op::Addi || now_op == Op::Slti || now_op == Op::Sltiu ||
             now_op == Op::Xori || now_op == Op::Ori || now_op == Op::Andi ||
             now_op == Op::Slli || now_op == Op::Srli || now_op == Op::Srai)
    {
      if (decoder_input.rs1_valid_ && decoder_input.imm_valid_)
      {
        // 发给rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rs_output_sign_.rd_ = decoder_input.rd_;
        auto rs1_query = rat.Query(decoder_input.rs1_);
        if (rs1_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(rs1_query.rob_id_).valid_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = rob.Query(rs1_query.rob_id_).value_;
          }
          else
          {
            rs_output_sign_.Qj_valid_ = true;
            rs_output_sign_.Qj_ = rs1_query.rob_id_;
          }
        }
        else
        {
          rs_output_sign_.Vj_valid_ = true;
          rs_output_sign_.Vj_ = registers.Read(decoder_input.rs1_);
        }
        rs_output_sign_.Vk_valid_ = true;
        rs_output_sign_.Vk_ = decoder_input.imm_;
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::WriteReg;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = true;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
    else if (now_op == Op::Lb || now_op == Op::Lh || now_op == Op::Lw ||
             now_op == Op::Lbu || now_op == Op::Lhu)
    {
      if (decoder_input.imm_valid_ && decoder_input.rs1_valid_)
      {
        // 发给lsq
        lsq_output_sign_.imm_valid_ = true;
        lsq_output_sign_.imm_ = decoder_input.imm_;
        lsq_output_sign_.op_ = now_op;
        lsq_output_sign_.rob_id_valid_ = true;
        lsq_output_sign_.rob_id_ = rob_id;
        auto lsq1_query = rat.Query(decoder_input.rs1_);
        if (lsq1_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == lsq1_query.rob_id_)
          {
            lsq_output_sign_.Vj_valid_ = true;
            lsq_output_sign_.Vj_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == lsq1_query.rob_id_)
          {
            lsq_output_sign_.Vj_valid_ = true;
            lsq_output_sign_.Vj_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(lsq1_query.rob_id_).valid_)
          {
            lsq_output_sign_.Vj_valid_ = true;
            lsq_output_sign_.Vj_ = rob.Query(lsq1_query.rob_id_).value_;
          }
          else
          {
            lsq_output_sign_.Qj_valid_ = true;
            lsq_output_sign_.Qj_ = lsq1_query.rob_id_;
          }
        }
        else
        {
          lsq_output_sign_.Vj_valid_ = true;
          lsq_output_sign_.Vj_ = registers.Read(decoder_input.rs1_);
        }
        lsq_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        lsq_output_sign_.rd_ = decoder_input.rd_;
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::WriteReg;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = true;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
    else if (now_op == Op::Sb || now_op == Op::Sh || now_op == Op::Sw)
    {
      if (decoder_input.imm_valid_ && decoder_input.rs1_valid_ &&
          decoder_input.rs2_valid_)
      {
        // 发给lsq
        lsq_output_sign_.imm_valid_ = true;
        lsq_output_sign_.imm_ = decoder_input.imm_;
        lsq_output_sign_.op_ = now_op;
        lsq_output_sign_.rob_id_valid_ = true;
        lsq_output_sign_.rob_id_ = rob_id;
        auto lsq1_query = rat.Query(decoder_input.rs1_);
        auto lsq2_query = rat.Query(decoder_input.rs2_);
        if (lsq1_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == lsq1_query.rob_id_)
          {
            lsq_output_sign_.Vj_valid_ = true;
            lsq_output_sign_.Vj_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == lsq1_query.rob_id_)
          {
            lsq_output_sign_.Vj_valid_ = true;
            lsq_output_sign_.Vj_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(lsq1_query.rob_id_).valid_)
          {
            lsq_output_sign_.Vj_valid_ = true;
            lsq_output_sign_.Vj_ = rob.Query(lsq1_query.rob_id_).value_;
          }
          else
          {
            lsq_output_sign_.Qj_valid_ = true;
            lsq_output_sign_.Qj_ = lsq1_query.rob_id_;
          }
        }
        else
        {
          lsq_output_sign_.Vj_valid_ = true;
          lsq_output_sign_.Vj_ = registers.Read(decoder_input.rs1_);
        }
        if (lsq2_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == lsq2_query.rob_id_)
          {
            lsq_output_sign_.Vk_valid_ = true;
            lsq_output_sign_.Vk_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == lsq2_query.rob_id_)
          {
            lsq_output_sign_.Vk_valid_ = true;
            lsq_output_sign_.Vk_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(lsq2_query.rob_id_).valid_)
          {
            lsq_output_sign_.Vk_valid_ = true;
            lsq_output_sign_.Vk_ = rob.Query(lsq2_query.rob_id_).value_;
          }
          else
          {
            lsq_output_sign_.Qk_valid_ = true;
            lsq_output_sign_.Qk_ = lsq2_query.rob_id_;
          }
        }
        else
        {
          lsq_output_sign_.Vk_valid_ = true;
          lsq_output_sign_.Vk_ = registers.Read(decoder_input.rs2_);
        }
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.rob_type = ROBType::WriteMem;
      }
    }
    else if (now_op == Op::Beq || now_op == Op::Bne || now_op == Op::Blt ||
             now_op == Op::Bge || now_op == Op::Bltu || now_op == Op::Bgeu)
    {
      if (decoder_input.pc_valid_ && decoder_input.imm_valid_ &&
          decoder_input.rs1_valid_ && decoder_input.rs2_valid_ &&
          decoder_input.predicted_next_pc_valid_)
      {
        // 发给 rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.offset_valid_ = true;
        rs_output_sign_.offset_ = decoder_input.imm_;
        rs_output_sign_.pc_valid_ = true;
        rs_output_sign_.pc_ = decoder_input.pc_;
        auto rs1_query = rat.Query(decoder_input.rs1_);
        auto rs2_query = rat.Query(decoder_input.rs2_);
        if (rs1_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(rs1_query.rob_id_).valid_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = rob.Query(rs1_query.rob_id_).value_;
          }
          else
          {
            rs_output_sign_.Qj_valid_ = true;
            rs_output_sign_.Qj_ = rs1_query.rob_id_;
          }
        }
        else
        {
          rs_output_sign_.Vj_valid_ = true;
          rs_output_sign_.Vj_ = registers.Read(decoder_input.rs1_);
        }
        if (rs2_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == rs2_query.rob_id_)
          {
            rs_output_sign_.Vk_valid_ = true;
            rs_output_sign_.Vk_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == rs2_query.rob_id_)
          {
            rs_output_sign_.Vk_valid_ = true;
            rs_output_sign_.Vk_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(rs2_query.rob_id_).valid_)
          {
            rs_output_sign_.Vk_valid_ = true;
            rs_output_sign_.Vk_ = rob.Query(rs2_query.rob_id_).value_;
          }
          else
          {
            rs_output_sign_.Qk_valid_ = true;
            rs_output_sign_.Qk_ = rs2_query.rob_id_;
          }
        }
        else
        {
          rs_output_sign_.Vk_valid_ = true;
          rs_output_sign_.Vk_ = registers.Read(decoder_input.rs2_);
        }
        // 发给 rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.predicted_pc_valid_ = true;
        rob_output_sign_.predicted_pc_ = decoder_input.predicted_next_pc_;
        rob_output_sign_.rob_type = ROBType::Branch;
      }
    }
    else if (now_op == Op::Jal)
    {
      if (decoder_input.imm_valid_ && decoder_input.pc_valid_ &&
          decoder_input.predicted_next_pc_valid_)
      {
        // 发给 rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.offset_valid_ = true;
        rs_output_sign_.offset_ = decoder_input.imm_;
        rs_output_sign_.pc_valid_ = true;
        rs_output_sign_.pc_ = decoder_input.pc_;
        rs_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rs_output_sign_.rd_ = decoder_input.rd_;
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.predicted_pc_valid_ = decoder_input.predicted_next_pc_valid_;
        rob_output_sign_.predicted_pc_ = decoder_input.predicted_next_pc_;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::Branch;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = decoder_input.rd_valid_;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
    else if (now_op == Op::Jalr)
    {
      if (decoder_input.imm_valid_ && decoder_input.pc_valid_ &&
          decoder_input.predicted_next_pc_valid_ && decoder_input.rs1_valid_)
      {
        // 发给 rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.Vk_valid_ = true;
        rs_output_sign_.Vk_ = decoder_input.imm_;
        rs_output_sign_.pc_valid_ = true;
        rs_output_sign_.pc_ = decoder_input.pc_;
        rs_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rs_output_sign_.rd_ = decoder_input.rd_;
        auto rs1_query = rat.Query(decoder_input.rs1_);
        if (rs1_query.rob_id_valid_)
        {
          if (current_state_.alu_cdb_input_.rob_id_valid_ &&
              current_state_.alu_cdb_input_.value_valid_ &&
              current_state_.alu_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.alu_cdb_input_.value_;
          }
          else if (current_state_.lsq_cdb_input_.rob_id_valid_ &&
                   current_state_.lsq_cdb_input_.value_valid_ &&
                   current_state_.lsq_cdb_input_.rob_id_ == rs1_query.rob_id_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = current_state_.lsq_cdb_input_.value_;
          }
          else if (rob.Query(rs1_query.rob_id_).valid_)
          {
            rs_output_sign_.Vj_valid_ = true;
            rs_output_sign_.Vj_ = rob.Query(rs1_query.rob_id_).value_;
          }
          else
          {
            rs_output_sign_.Qj_valid_ = true;
            rs_output_sign_.Qj_ = rs1_query.rob_id_;
          }
        }
        else
        {
          rs_output_sign_.Vj_valid_ = true;
          rs_output_sign_.Vj_ = registers.Read(decoder_input.rs1_);
        }
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.predicted_pc_valid_ = true;
        rob_output_sign_.predicted_pc_ = decoder_input.predicted_next_pc_;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::Branch;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = decoder_input.rd_valid_;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
    else if (now_op == Op::Auipc)
    {
      if (decoder_input.imm_valid_ && decoder_input.pc_valid_)
      {
        // 发给rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.Vk_valid_ = true;
        rs_output_sign_.Vk_ = decoder_input.imm_; // 解码时已经左移
        rs_output_sign_.pc_valid_ = true;
        rs_output_sign_.pc_ = decoder_input.pc_;
        rs_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rs_output_sign_.rd_ = decoder_input.rd_;
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::WriteReg;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = decoder_input.rd_valid_;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
    else if (now_op == Op::Lui)
    {
      if (decoder_input.imm_valid_)
      {
        // 发给rs
        rs_output_sign_.op_ = now_op;
        rs_output_sign_.rob_id_valid_ = true;
        rs_output_sign_.rob_id_ = rob_id;
        rs_output_sign_.Vk_valid_ = true;
        rs_output_sign_.Vk_ = decoder_input.imm_;
        rs_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rs_output_sign_.rd_ = decoder_input.rd_;
        // 发给rob
        rob_output_sign_.rob_id_valid_ = true;
        rob_output_sign_.rob_id_ = rob_id;
        rob_output_sign_.rd_valid_ = decoder_input.rd_valid_;
        rob_output_sign_.rd_ = decoder_input.rd_;
        rob_output_sign_.rob_type = ROBType::WriteReg;
        // 发给rat
        if (decoder_input.rd_valid_)
        {
          rat_output_sign_.register_id_valid_ = decoder_input.rd_valid_;
          rat_output_sign_.register_id_ = decoder_input.rd_;
          rat_output_sign_.rob_id_valid_ = true;
          rat_output_sign_.rob_id_ = rob_id;
        }
      }
    }
  }
}

void Issue::UpdateNext() // 利用本周期input计算 next_state_
{
  next_state_.flush_input_ = flush_input_sign_;
  next_state_.alu_cdb_input_ = alu_cdb_input_sign_;
  next_state_.lsq_cdb_input_ = lsq_cdb_input_sign_;
  if (flush_input_sign_.need_flush_)
  {
    next_state_ = IssueState{
        .flush_input_ = flush_input_sign_,
    };
    return;
  }
  if (!ready_)
  {
    next_state_.decoder_input_ = current_state_.decoder_input_;
    return;
  }
  next_state_ = IssueState{
      .decoder_input_ = decoder_input_sign_,
      .alu_cdb_input_ = alu_cdb_input_sign_,
      .lsq_cdb_input_ = lsq_cdb_input_sign_,
      .flush_input_ = flush_input_sign_,
  };
}
