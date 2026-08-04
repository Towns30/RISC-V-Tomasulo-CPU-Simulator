#include "RAT.hpp"

RAT::RAT(const IssueToRATSign &issue_input_sign,
         const ROBToRATSign &rob_input_sign,
         const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
             &flush_input_sign)
    : issue_input_sign_(issue_input_sign), rob_input_sign_(rob_input_sign),
      flush_input_sign_(flush_input_sign)
{
}

RATQueryResult RAT::Query(uint8_t register_id)
{
  if (register_id == 0)
  {
    return {};
  }
  if (current_state_.rat_map_[register_id].is_empty_) // 无依赖
  {
    return RATQueryResult{.rob_id_valid_ = false, .rob_id_ = 0};
  }
  return RATQueryResult{.rob_id_valid_ = true,
                        .rob_id_ =
                            current_state_.rat_map_[register_id].rob_id_};
}

void RAT::UpdateCurrent() { current_state_ = next_state_; }

void RAT::Execute() // 利用current发送output
{
}

void RAT::UpdateNext() // 利用本周期input计算 next_state_
{
  next_state_ = current_state_;
  if (flush_input_sign_.need_flush_) // 需要flush则将下周期依赖状态清空
  {
    for (int i = 0; i <= 31; i++)
    {
      next_state_.rat_map_[i] = RobID{.is_empty_ = true, .rob_id_ = 0};
    }
    return;
  }
  if (rob_input_sign_.commit_valid_ &&
      next_state_.rat_map_[rob_input_sign_.rd_].rob_id_ ==
          rob_input_sign_.rob_id_)
  {
    next_state_.rat_map_[rob_input_sign_.rd_].is_empty_ = true;
    next_state_.rat_map_[rob_input_sign_.rd_].rob_id_ = 0;
  }
  if (issue_input_sign_.register_id_valid_ && issue_input_sign_.rob_id_valid_ && issue_input_sign_.register_id_ != 0)
  {
    next_state_.rat_map_[issue_input_sign_.register_id_].is_empty_ = false;
    next_state_.rat_map_[issue_input_sign_.register_id_].rob_id_ =
        issue_input_sign_.rob_id_;
  }
}
