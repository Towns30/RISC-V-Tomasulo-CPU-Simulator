#include "ROB.hpp"

ROB::ROB(
    const IssueToROBSign &issue_input_sign, const LSQToROBSign &lsq_input_sign,
    const ALUToCDBSign &alu_cdb_input_sign,
    const LSQToCDBSign &lsq_cdb_input_sign,
    const MemToROBSign &memory_input_sign,
    ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign &flush_output_sign,
    ROBToFetchFlushSign &fetch_flush_output_sign, ROBToLSQSign &lsq_output_sign,
    ROBToRATSign &rat_output_sign, ROBToRegisterSign &register_output_sign)
    : issue_input_sign_(issue_input_sign), lsq_input_sign_(lsq_input_sign),
      alu_cdb_input_sign_(alu_cdb_input_sign),
      lsq_cdb_input_sign_(lsq_cdb_input_sign),
      memory_input_sign_(memory_input_sign),
      flush_output_sign_(flush_output_sign),
      fetch_flush_output_sign_(fetch_flush_output_sign),
      lsq_output_sign_(lsq_output_sign), rat_output_sign_(rat_output_sign),
      register_output_sign_(register_output_sign)
{
}

bool ROB::Ready()
{
  if ((current_state_.tail_ + 1) % 16 == current_state_.head_)
  {
    return false;
  }
  return true;
}

ROBQueryIDResult ROB::NextId()
{
  if ((current_state_.tail_ + 1) % 16 == current_state_.head_)
  {
    return {};
  }
  return ROBQueryIDResult{.valid_ = true, .rob_id_ = current_state_.tail_};
}

ROBQueryResult ROB::Query(uint8_t rob_id)
{
  if (current_state_.rob_infos_[rob_id].ready_ &&
      current_state_.rob_infos_[rob_id].rd_value_valid_)
  {
    return ROBQueryResult{
        .valid_ = true, .value_ = current_state_.rob_infos_[rob_id].rd_value_};
  }
  return {};
}

void ROB::UpdateCurrent() { current_state_ = next_state_; }

void ROB::Execute() // 利用current发送output，以及修改next
{
  next_state_ = current_state_;
  if (current_state_.head_ == current_state_.tail_) // empty
  {
    return;
  }
  ROBInfo rob_info = current_state_.rob_infos_[current_state_.head_];
  if (!rob_info.ready_) // 队头没准备好
  {
    return;
  }
  uint8_t rob_id = current_state_.head_;
  switch (rob_info.rob_type)
  {
  case ROBType::WriteReg:
  {
    if (!rob_info.rd_value_valid_)
    {
      return;
    }
    if (rob_info.rd_valid_)
    {
      // To Register
      register_output_sign_.rd_valid_ = true;
      register_output_sign_.rd_ = rob_info.rd_;
      register_output_sign_.value_valid_ = true;
      register_output_sign_.value_ = rob_info.rd_value_;
      // To RAT
      rat_output_sign_.commit_valid_ = true;
      rat_output_sign_.rd_ = rob_info.rd_;
      rat_output_sign_.rob_id_ = rob_id;
    }
    // 出队
    next_state_.head_ = (next_state_.head_ + 1) % 16;
    break;
  }
  case ROBType::WriteMem:
  {
    if (!rob_info.addr_valid_ || !rob_info.addr_value_valid_ ||
        !rob_info.write_mem_state_valid_)
    {
      return;
    }
    if (rob_info.write_mem_state_ ==
        WriteMemState::NotDone) // 没写完，什么都不能做
    {
      return;
    }
    lsq_output_sign_.rob_id_valid_ = true;
    lsq_output_sign_.rob_id_ = rob_id;
    lsq_output_sign_.write_enabled_ = true;
    next_state_.rob_infos_[next_state_.head_].write_mem_state_ =
        WriteMemState::NotDone;
    break;
  }
  case ROBType::Branch:
  {
    if (!rob_info.predict_pc_valid_ || !rob_info.real_pc_valid_)
    {
      return;
    }
    // JAL/JALR 如果需要写 rd，还必须等 PC + 4 返回
    if (rob_info.rd_valid_ && !rob_info.rd_value_valid_)
    {
      return;
    }
    if (rob_info.rd_valid_)
    {
      // To Register
      register_output_sign_.rd_valid_ = true;
      register_output_sign_.rd_ = rob_info.rd_;
      register_output_sign_.value_valid_ = true;
      register_output_sign_.value_ = rob_info.rd_value_;
      // To RAT
      rat_output_sign_.commit_valid_ = true;
      rat_output_sign_.rd_ = rob_info.rd_;
      rat_output_sign_.rob_id_ = rob_id;
    }
    if (rob_info.predict_pc_ != rob_info.real_pc_) // 分支预测错误
    {
      flush_output_sign_.need_flush_ = true;
      fetch_flush_output_sign_.need_flush_ = true;
      fetch_flush_output_sign_.real_pc_valid_ = true;
      fetch_flush_output_sign_.real_pc_ = rob_info.real_pc_;
      next_state_ = ROBState{};
      return;
    }
    // 出队
    next_state_.head_ = (next_state_.head_ + 1) % 16;
    break;
  }
  }
}

void ROB::UpdateNext() // 利用本周期input计算 next_state_
{
  if (flush_output_sign_.need_flush_) // 检测到flush，下周期状态清空
  {
    return;
  }
  if (lsq_input_sign_.addr_valid_ && lsq_input_sign_.rob_id_valid_ &&
      lsq_input_sign_.value_valid_)
  {
    auto &modify_rob_info = next_state_.rob_infos_[lsq_input_sign_.rob_id_];
    modify_rob_info.addr_valid_ = true;
    modify_rob_info.addr_ = lsq_input_sign_.addr_;
    modify_rob_info.addr_value_valid_ = true;
    modify_rob_info.addr_value_ = lsq_input_sign_.value_;
    modify_rob_info.write_mem_state_valid_ = true;
    modify_rob_info.write_mem_state_ = WriteMemState::NoWrite;
    modify_rob_info.ready_ = true;
  }
  if (memory_input_sign_.is_written_ && memory_input_sign_.rob_id_valid_)
  {
    next_state_.head_ = (next_state_.head_ + 1) % 16; // 内存写完出队
  }
  if (alu_cdb_input_sign_.rob_id_valid_)
  {
    if (alu_cdb_input_sign_.real_pc_valid_)
    {
      next_state_.rob_infos_[alu_cdb_input_sign_.rob_id_].real_pc_valid_ = true;
      next_state_.rob_infos_[alu_cdb_input_sign_.rob_id_].real_pc_ =
          alu_cdb_input_sign_.real_pc_;
      next_state_.rob_infos_[alu_cdb_input_sign_.rob_id_].ready_ = true;
    }
    if (alu_cdb_input_sign_.value_valid_)
    {
      next_state_.rob_infos_[alu_cdb_input_sign_.rob_id_].rd_value_valid_ = true;
      next_state_.rob_infos_[alu_cdb_input_sign_.rob_id_].rd_value_ =
          alu_cdb_input_sign_.value_;
      next_state_.rob_infos_[alu_cdb_input_sign_.rob_id_].ready_ = true;
    }
  }
  if (lsq_cdb_input_sign_.rob_id_valid_ &&
      lsq_cdb_input_sign_.value_valid_)
  {
    next_state_.rob_infos_[lsq_cdb_input_sign_.rob_id_].rd_value_valid_ = true;
    next_state_.rob_infos_[lsq_cdb_input_sign_.rob_id_].rd_value_ =
        lsq_cdb_input_sign_.value_;
    next_state_.rob_infos_[lsq_cdb_input_sign_.rob_id_].ready_ = true;
  }
  if (issue_input_sign_.rob_id_valid_)
  {
    switch (issue_input_sign_.rob_type)
    {
    case ROBType::WriteReg:
    {
      next_state_.rob_infos_[issue_input_sign_.rob_id_] =
          ROBInfo{.rob_type = ROBType::WriteReg,
                  .rd_valid_ = issue_input_sign_.rd_valid_,
                  .rd_ = issue_input_sign_.rd_};
      next_state_.tail_ = (next_state_.tail_ + 1) % 16; // 入队
      break;
    }
    case ROBType::WriteMem:
    {
      next_state_.rob_infos_[issue_input_sign_.rob_id_] =
          ROBInfo{.rob_type = ROBType::WriteMem,
                  .write_mem_state_valid_ = true,
                  .write_mem_state_ = WriteMemState::NoWrite};
      next_state_.tail_ = (next_state_.tail_ + 1) % 16; // 入队
      break;
    }
    case ROBType::Branch:
    {
      next_state_.rob_infos_[issue_input_sign_.rob_id_] =
          ROBInfo{.rob_type = ROBType::Branch,
                  .predict_pc_valid_ = issue_input_sign_.predicted_pc_valid_,
                  .predict_pc_ = issue_input_sign_.predicted_pc_};
      if (issue_input_sign_.rd_valid_)
      {
        next_state_.rob_infos_[issue_input_sign_.rob_id_].rd_valid_ = true;
        next_state_.rob_infos_[issue_input_sign_.rob_id_].rd_ =
            issue_input_sign_.rd_;
      }
      next_state_.tail_ = (next_state_.tail_ + 1) % 16; // 入队
      break;
    }
    }
  }
}
