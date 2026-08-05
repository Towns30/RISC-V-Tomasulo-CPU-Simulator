#include "RS.hpp"

RS::RS(const IssueToRSSign &issue_input_sign, RSToALUSign &alu_output_sign,
       const ALUToCDBSign &alu_cdb_input_sign,
       const LSQToCDBSign &lsq_cdb_input_sign,
       const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
           &flush_input_sign)
    : issue_input_sign_(issue_input_sign), alu_output_sign_(alu_output_sign),
      alu_cdb_input_sign_(alu_cdb_input_sign),
      lsq_cdb_input_sign_(lsq_cdb_input_sign),
      flush_input_sign_(flush_input_sign)
{
}

bool RS::Ready()
{
  for (int i = 0; i < 16; i++)
  {
    if (!current_state_.rs_infos_[i].info_valid_) // 有空位
    {
      return true;
    }
  }
  return false;
}

void RS::UpdateCurrent() { current_state_ = next_state_; }

void RS::Execute() // 利用current发送output，以及修改next
{
  next_state_ = current_state_;
  if (current_state_.flush_input_.need_flush_)
  {
    return;
  }
  for (int i = 0; i < 16; i++)
  {
    bool has_found = false;
    if (current_state_.rs_infos_[i].info_valid_) // 此行有效
    {
      Op now_op = current_state_.rs_infos_[i].op_;
      auto info = current_state_.rs_infos_[i];
      if (now_op == Op::Add || now_op == Op::Sub || now_op == Op::And ||
          now_op == Op::Or || now_op == Op::Xor || now_op == Op::Sll ||
          now_op == Op::Srl || now_op == Op::Sra || now_op == Op::Slt ||
          now_op == Op::Sltu ||
          now_op == Op::Addi || now_op == Op::Slti || now_op == Op::Sltiu ||
          now_op == Op::Xori || now_op == Op::Ori || now_op == Op::Andi ||
          now_op == Op::Slli || now_op == Op::Srli || now_op == Op::Srai)
      {
        if (info.vj_valid_ && info.vk_valid_)
        {
          alu_output_sign_.op_ = now_op;
          alu_output_sign_.Vj_valid_ = true;
          alu_output_sign_.Vj_ = info.vj_;
          alu_output_sign_.Vk_valid_ = true;
          alu_output_sign_.Vk_ = info.vk_;
          alu_output_sign_.rob_id_valid_ = true;
          alu_output_sign_.rob_id_ = info.rob_id_;
          has_found = true;
        }
      }
      else if (now_op == Op::Beq || now_op == Op::Bne ||
               now_op == Op::Blt || now_op == Op::Bge ||
               now_op == Op::Bltu || now_op == Op::Bgeu)
      {
        if (info.vj_valid_ && info.vk_valid_ && info.pc_valid_ && info.offset_valid_)
        {
          alu_output_sign_.op_ = now_op;
          alu_output_sign_.Vj_valid_ = true;
          alu_output_sign_.Vj_ = info.vj_;
          alu_output_sign_.Vk_valid_ = true;
          alu_output_sign_.Vk_ = info.vk_;
          alu_output_sign_.offset_valid_ = true;
          alu_output_sign_.offset_ = info.offset_;
          alu_output_sign_.rob_id_valid_ = true;
          alu_output_sign_.rob_id_ = info.rob_id_;
          alu_output_sign_.pc_valid_ = true;
          alu_output_sign_.pc_ = info.pc_;
          has_found = true;
        }
      }
      else if (now_op == Op::Jal)
      {
        if (info.pc_valid_ && info.offset_valid_)
        {
          alu_output_sign_.op_ = now_op;
          alu_output_sign_.rob_id_valid_ = true;
          alu_output_sign_.rob_id_ = info.rob_id_;
          alu_output_sign_.pc_valid_ = true;
          alu_output_sign_.pc_ = info.pc_;
          alu_output_sign_.offset_valid_ = true;
          alu_output_sign_.offset_ = info.offset_;
          has_found = true;
        }
      }
      else if (now_op == Op::Jalr)
      {
        if (info.pc_valid_ && info.vj_valid_ && info.vk_valid_)
        {
          alu_output_sign_.op_ = now_op;
          alu_output_sign_.rob_id_valid_ = true;
          alu_output_sign_.rob_id_ = info.rob_id_;
          alu_output_sign_.pc_valid_ = true;
          alu_output_sign_.pc_ = info.pc_;
          alu_output_sign_.Vj_valid_ = true;
          alu_output_sign_.Vj_ = info.vj_;
          alu_output_sign_.Vk_valid_ = true;
          alu_output_sign_.Vk_ = info.vk_;
          has_found = true;
        }
      }
      else if (now_op == Op::Auipc)
      {
        if (info.pc_valid_ && info.vk_valid_)
        {
          alu_output_sign_.op_ = now_op;
          alu_output_sign_.rob_id_valid_ = true;
          alu_output_sign_.rob_id_ = info.rob_id_;
          alu_output_sign_.pc_valid_ = true;
          alu_output_sign_.pc_ = info.pc_;
          alu_output_sign_.Vk_valid_ = true;
          alu_output_sign_.Vk_ = info.vk_;
          has_found = true;
        }
      }
      else if (now_op == Op::Lui)
      {
        if (info.vk_valid_)
        {
          alu_output_sign_.op_ = now_op;
          alu_output_sign_.rob_id_valid_ = true;
          alu_output_sign_.rob_id_ = info.rob_id_;
          alu_output_sign_.Vk_valid_ = true;
          alu_output_sign_.Vk_ = info.vk_;
          has_found = true;
        }
      }
    }
    if (has_found)
    {
      next_state_.rs_infos_[i].info_valid_ = false;
      return;
    }
  }
}

void RS::UpdateNext() // 利用本周期input计算 next_state_
{
  next_state_.flush_input_ = flush_input_sign_;
  if (flush_input_sign_.need_flush_)
  {
    for (int i = 0; i < 16; i++)
    {
      next_state_.rs_infos_[i].info_valid_ = false;
    }
    return;
  }
  if (issue_input_sign_.rob_id_valid_) // issue输入，添加一项
  {
    uint8_t pos = 0;
    // 找第一个空位
    for (int i = 0; i < 16; i++)
    {
      if (!next_state_.rs_infos_[i].info_valid_)
      {
        pos = i;
        break;
      }
    }
    auto &rs_info = next_state_.rs_infos_[pos];
    rs_info = RSInfo{};
    rs_info.info_valid_ = true;
    rs_info.rob_id_ = issue_input_sign_.rob_id_;
    rs_info.op_ = issue_input_sign_.op_;
    rs_info.pc_valid_ = issue_input_sign_.pc_valid_;
    rs_info.pc_ = issue_input_sign_.pc_;
    rs_info.offset_valid_ = issue_input_sign_.offset_valid_;
    rs_info.offset_ = issue_input_sign_.offset_;
    rs_info.rd_valid_ = issue_input_sign_.rd_valid_;
    rs_info.rd_ = issue_input_sign_.rd_;
    rs_info.vj_valid_ = issue_input_sign_.Vj_valid_;
    rs_info.vj_ = issue_input_sign_.Vj_;
    rs_info.vk_valid_ = issue_input_sign_.Vk_valid_;
    rs_info.vk_ = issue_input_sign_.Vk_;
    rs_info.qj_valid_ = issue_input_sign_.Qj_valid_;
    rs_info.qj_ = issue_input_sign_.Qj_;
    rs_info.qk_valid_ = issue_input_sign_.Qk_valid_;
    rs_info.qk_ = issue_input_sign_.Qk_;
  }
  if (alu_cdb_input_sign_.rob_id_valid_ &&
      alu_cdb_input_sign_.value_valid_) // 只有算出值才对RS有意义
  {
    for (int i = 0; i < 16; i++)
    {
      if (next_state_.rs_infos_[i].info_valid_)
      {
        if (next_state_.rs_infos_[i].qj_valid_ &&
            next_state_.rs_infos_[i].qj_ == alu_cdb_input_sign_.rob_id_)
        {
          next_state_.rs_infos_[i].qj_valid_ = false;
          next_state_.rs_infos_[i].vj_valid_ = true;
          next_state_.rs_infos_[i].vj_ = alu_cdb_input_sign_.value_;
        }
        if (next_state_.rs_infos_[i].qk_valid_ &&
            next_state_.rs_infos_[i].qk_ == alu_cdb_input_sign_.rob_id_)
        {
          next_state_.rs_infos_[i].qk_valid_ = false;
          next_state_.rs_infos_[i].vk_valid_ = true;
          next_state_.rs_infos_[i].vk_ = alu_cdb_input_sign_.value_;
        }
      }
    }
  }
  if (lsq_cdb_input_sign_.rob_id_valid_ &&
      lsq_cdb_input_sign_.value_valid_) // lsq的广播
  {
    for (int i = 0; i < 16; i++)
    {
      if (next_state_.rs_infos_[i].info_valid_)
      {
        if (next_state_.rs_infos_[i].qj_valid_ &&
            next_state_.rs_infos_[i].qj_ == lsq_cdb_input_sign_.rob_id_)
        {
          next_state_.rs_infos_[i].qj_valid_ = false;
          next_state_.rs_infos_[i].vj_valid_ = true;
          next_state_.rs_infos_[i].vj_ = lsq_cdb_input_sign_.value_;
        }
        if (next_state_.rs_infos_[i].qk_valid_ &&
            next_state_.rs_infos_[i].qk_ == lsq_cdb_input_sign_.rob_id_)
        {
          next_state_.rs_infos_[i].qk_valid_ = false;
          next_state_.rs_infos_[i].vk_valid_ = true;
          next_state_.rs_infos_[i].vk_ = lsq_cdb_input_sign_.value_;
        }
      }
    }
  }
}
