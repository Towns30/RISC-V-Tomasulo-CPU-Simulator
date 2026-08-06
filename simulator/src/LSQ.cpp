#include "LSQ.hpp"

LSQ::LSQ(const IssueToLSQSign &issue_input_sign,
         LSQToMemSign &memory_output_sign, LSQToCDBSign &cdb_output_sign,
         LSQToROBSign &rob_output_sign, const ALUToCDBSign &alu_cdb_input_sign,
         const MemToLSQSign &memory_input_sign,
         const ROBToLSQSign &rob_input_sign,
         const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
             &flush_input_sign)
    : issue_input_sign_(issue_input_sign),
      memory_output_sign_(memory_output_sign),
      cdb_output_sign_(cdb_output_sign), rob_output_sign_(rob_output_sign),
      alu_cdb_input_sign_(alu_cdb_input_sign),
      memory_input_sign_(memory_input_sign), rob_input_sign_(rob_input_sign),
      flush_input_sign_(flush_input_sign)
{
}

bool LSQ::Ready() const
{
  return !((current_state_.tail_ + 1) % 16 == current_state_.head_);
}

void LSQ::UpdateCurrent() { current_state_ = next_state_; }

void LSQ::Execute() // 利用current发送output，以及修改next
{
  if (current_state_.head_ != current_state_.tail_)
  { // 乱序做地址加法
    for (int i = current_state_.head_; i != current_state_.tail_;
         i = (i + 1) % 16)
    {
      bool has_found = false;
      if (current_state_.lsq_infos_[i].info_valid_) // 此行有效
      {
        auto lsq_info = current_state_.lsq_infos_[i];
        if (lsq_info.vj_valid_ && lsq_info.imm_valid_ &&
            !lsq_info.addr_valid_ && lsq_info.state_ == 0)
        {
          next_state_.lsq_infos_[i].addr_valid_ = true;
          next_state_.lsq_infos_[i].addr_ = lsq_info.vj_ + lsq_info.imm_;
          next_state_.lsq_infos_[i].state_ = 1;
          has_found = true;
        }
      }
      if (has_found)
      {
        break;
      }
    }
    // 顺序做后续操作
    auto lsq_head_info = current_state_.lsq_infos_[current_state_.head_];
    if (lsq_head_info.lsq_type_ == LSQType::Store)
    {
      if (lsq_head_info.state_ == 1)
      {
        if (lsq_head_info.vk_valid_ && !lsq_head_info.qk_valid_ &&
            lsq_head_info.addr_valid_)
        {
          rob_output_sign_.addr_valid_ = true;
          rob_output_sign_.addr_ = lsq_head_info.addr_;
          rob_output_sign_.rob_id_valid_ = true;
          rob_output_sign_.rob_id_ = lsq_head_info.rob_id_;
          rob_output_sign_.value_valid_ = true;
          rob_output_sign_.value_ = lsq_head_info.vk_;
          next_state_.lsq_infos_[next_state_.head_].state_ = 2; // 进入下一状态
        }
      }
      else if (lsq_head_info.state_ == 3)
      {
        memory_output_sign_.rob_id_valid_ = true;
        memory_output_sign_.rob_id_ = lsq_head_info.rob_id_;
        memory_output_sign_.addr_valid_ = true;
        memory_output_sign_.addr_ = lsq_head_info.addr_;
        memory_output_sign_.value_valid_ = true;
        memory_output_sign_.value_ = lsq_head_info.vk_;
        memory_output_sign_.op_ = lsq_head_info.op_;
        switch (lsq_head_info.op_)
        {
        case Op::Sb:
        {
          memory_output_sign_.bytes_ = 1;
          break;
        }
        case Op::Sh:
        {
          memory_output_sign_.bytes_ = 2;
          break;
        }
        case Op::Sw:
        {
          memory_output_sign_.bytes_ = 4;
          break;
        }
        default:
        {
          memory_output_sign_ = {};
          return;
        }
        }
        next_state_.lsq_infos_[next_state_.head_].state_ =
            4; // 进入等待写好信号状态
      }
    }
    else
    {
      if (lsq_head_info.state_ == 1)
      {
        if (lsq_head_info.addr_valid_)
        {
          memory_output_sign_.addr_valid_ = true;
          memory_output_sign_.addr_ = lsq_head_info.addr_;
          memory_output_sign_.rob_id_valid_ = true;
          memory_output_sign_.rob_id_ = lsq_head_info.rob_id_;
          memory_output_sign_.op_ = lsq_head_info.op_;
          switch (lsq_head_info.op_)
          {
          case Op::Lb:
          {
            memory_output_sign_.bytes_ = 1;
            memory_output_sign_.is_signed_ = true;
            break;
          }
          case Op::Lbu:
          {
            memory_output_sign_.bytes_ = 1;
            memory_output_sign_.is_signed_ = false;
            break;
          }
          case Op::Lh:
          {
            memory_output_sign_.bytes_ = 2;
            memory_output_sign_.is_signed_ = true;
            break;
          }
          case Op::Lhu:
          {
            memory_output_sign_.bytes_ = 2;
            memory_output_sign_.is_signed_ = false;
            break;
          }
          case Op::Lw:
          {
            memory_output_sign_.bytes_ = 4;
            memory_output_sign_.is_signed_ = true;
            break;
          }
          default:
          {
            memory_output_sign_ = {};
            return;
          }
          }
          next_state_.lsq_infos_[next_state_.head_].state_ = 2;
        }
      }
      if (lsq_head_info.state_ ==
          2) // 上一周期读完了，这一周期要发cdb和rob ready
      {
        if (current_state_.memory_input_.rob_id_valid_ &&
            current_state_.memory_input_.value_valid_ &&
            current_state_.memory_input_.rob_id_ == lsq_head_info.rob_id_)
        {
          cdb_output_sign_.rob_id_valid_ = true;
          cdb_output_sign_.rob_id_ = current_state_.memory_input_.rob_id_;
          cdb_output_sign_.value_valid_ = true;
          cdb_output_sign_.value_ = current_state_.memory_input_.value_;
          rob_output_sign_.addr_valid_ = true;
          rob_output_sign_.addr_ = lsq_head_info.addr_;
          rob_output_sign_.rob_id_valid_ = true;
          rob_output_sign_.rob_id_ = lsq_head_info.rob_id_;
          rob_output_sign_.value_valid_ = true;
          rob_output_sign_.value_ = current_state_.memory_input_.value_;
          // 出队
          next_state_.lsq_infos_[next_state_.head_].info_valid_ = false;
          next_state_.head_ = (next_state_.head_ + 1) % 16;
        }
      }
    }
  }
}

void LSQ::UpdateNext() // 利用本周期input计算 next_state_
{
  if (flush_input_sign_.need_flush_) // 清空
  {
    for (int i = 0; i < 16; i++)
    {
      next_state_.lsq_infos_[i].info_valid_ = false;
    }
    next_state_.head_ = next_state_.tail_ = 0;
    return;
  }
  next_state_.memory_input_ = memory_input_sign_;
  if (issue_input_sign_.rob_id_valid_)
  {
    if (issue_input_sign_.op_ == Op::Lb || issue_input_sign_.op_ == Op::Lbu ||
        issue_input_sign_.op_ == Op::Lh || issue_input_sign_.op_ == Op::Lhu ||
        issue_input_sign_.op_ == Op::Lw)
    {
      next_state_.lsq_infos_[next_state_.tail_] =
          LSQInfo{.info_valid_ = true,
                  .op_ = issue_input_sign_.op_,
                  .lsq_type_ = LSQType::Load,
                  .rob_id_ = issue_input_sign_.rob_id_,
                  .vj_valid_ = issue_input_sign_.Vj_valid_,
                  .vj_ = issue_input_sign_.Vj_,
                  .qj_valid_ = issue_input_sign_.Qj_valid_,
                  .qj_ = issue_input_sign_.Qj_,
                  .imm_valid_ = issue_input_sign_.imm_valid_,
                  .imm_ = issue_input_sign_.imm_,
                  .rd_valid_ = issue_input_sign_.rd_valid_,
                  .rd_ = issue_input_sign_.rd_};
      next_state_.tail_ = (next_state_.tail_ + 1) % 16;
    }
    else
    {
      next_state_.lsq_infos_[next_state_.tail_] =
          LSQInfo{.info_valid_ = true,
                  .op_ = issue_input_sign_.op_,
                  .lsq_type_ = LSQType::Store,
                  .rob_id_ = issue_input_sign_.rob_id_,
                  .vj_valid_ = issue_input_sign_.Vj_valid_,
                  .vj_ = issue_input_sign_.Vj_,
                  .vk_valid_ = issue_input_sign_.Vk_valid_,
                  .vk_ = issue_input_sign_.Vk_,
                  .qj_valid_ = issue_input_sign_.Qj_valid_,
                  .qj_ = issue_input_sign_.Qj_,
                  .qk_valid_ = issue_input_sign_.Qk_valid_,
                  .qk_ = issue_input_sign_.Qk_,
                  .imm_valid_ = issue_input_sign_.imm_valid_,
                  .imm_ = issue_input_sign_.imm_};
      next_state_.tail_ = (next_state_.tail_ + 1) % 16;
    }
  }
  if (alu_cdb_input_sign_.rob_id_valid_ && alu_cdb_input_sign_.value_valid_)
  {
    for (int i = next_state_.head_; i != next_state_.tail_; i = (i + 1) % 16)
    {
      if (next_state_.lsq_infos_[i].qj_valid_ &&
          next_state_.lsq_infos_[i].qj_ == alu_cdb_input_sign_.rob_id_)
      {
        next_state_.lsq_infos_[i].qj_valid_ = false;
        next_state_.lsq_infos_[i].vj_valid_ = true;
        next_state_.lsq_infos_[i].vj_ = alu_cdb_input_sign_.value_;
      }
      if (next_state_.lsq_infos_[i].qk_valid_ &&
          next_state_.lsq_infos_[i].qk_ == alu_cdb_input_sign_.rob_id_)
      {
        next_state_.lsq_infos_[i].qk_valid_ = false;
        next_state_.lsq_infos_[i].vk_valid_ = true;
        next_state_.lsq_infos_[i].vk_ = alu_cdb_input_sign_.value_;
      }
    }
  }
  if (memory_input_sign_.rob_id_valid_ &&
      memory_input_sign_.value_valid_) // mem读完
  {
    // 用memory的返回更新依赖关系
    for (int i = next_state_.head_; i != next_state_.tail_; i = (i + 1) % 16)
    {
      if (next_state_.lsq_infos_[i].qj_valid_ &&
          next_state_.lsq_infos_[i].qj_ == memory_input_sign_.rob_id_)
      {
        next_state_.lsq_infos_[i].qj_valid_ = false;
        next_state_.lsq_infos_[i].vj_valid_ = true;
        next_state_.lsq_infos_[i].vj_ = memory_input_sign_.value_;
      }
      if (next_state_.lsq_infos_[i].qk_valid_ &&
          next_state_.lsq_infos_[i].qk_ == memory_input_sign_.rob_id_)
      {
        next_state_.lsq_infos_[i].qk_valid_ = false;
        next_state_.lsq_infos_[i].vk_valid_ = true;
        next_state_.lsq_infos_[i].vk_ = memory_input_sign_.value_;
      }
    }
  }
  if (memory_input_sign_.is_written_ && memory_input_sign_.rob_id_valid_ &&
      next_state_.lsq_infos_[next_state_.head_].state_ == 4 &&
      memory_input_sign_.rob_id_ ==
          next_state_.lsq_infos_[next_state_.head_].rob_id_)
  {
    next_state_.lsq_infos_[next_state_.head_].info_valid_ = false;
    next_state_.head_ = (next_state_.head_ + 1) % 16; // 出队
  }
  if (memory_input_sign_.rob_id_valid_ && memory_input_sign_.value_valid_ &&
      memory_input_sign_.rob_id_ ==
          next_state_.lsq_infos_[next_state_.head_].rob_id_) // 读完了
  {
    next_state_.lsq_infos_[next_state_.head_].state_ = 2;
  }
  if (rob_input_sign_.rob_id_valid_ && rob_input_sign_.write_enabled_ &&
      next_state_.lsq_infos_[next_state_.head_].state_ == 2 &&
      rob_input_sign_.rob_id_ ==
          next_state_.lsq_infos_[next_state_.head_].rob_id_)
  {
    next_state_.lsq_infos_[next_state_.head_].state_ = 3;
  }
}
