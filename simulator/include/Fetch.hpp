#ifndef FETCH_HPP
#define FETCH_HPP

#include "Sign.hpp"

struct FetchState
{
  uint32_t now_pc_ = 0; // 当周期需要申请的pc
  MemToFetchSign memory_input_;
  ROBToFetchFlushSign flush_input_;

  auto operator<=>(const FetchState &) const = default;
};

class Fetch
{
public:
  Fetch(FetchToMemSign &memory_output_sign,
        const MemToFetchSign &memory_input_sign,
        FetchToDecoderSign &decoder_output_sign,
        const ROBToFetchFlushSign &flush_input_sign);

  void SetReady(bool ready);
  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  FetchToMemSign &memory_output_sign_;
  const MemToFetchSign &memory_input_sign_;
  FetchToDecoderSign &decoder_output_sign_;
  const ROBToFetchFlushSign &flush_input_sign_;
  FetchState current_state_;
  FetchState next_state_;
  bool ready_ = true;
};

#endif
