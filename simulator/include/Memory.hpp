#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "Sign.hpp"

#include <iosfwd>

struct WritingReadingState
{
  bool is_writing_ = false;
  uint8_t writing_state_ = 0; // 0, 1, 2, 2 就释放了，其实不会出现 2
  uint8_t writing_rob_id_ = 0;
  uint32_t writing_addr_ = 0;
  uint32_t writing_value_ = 0;
  uint32_t writing_bytes_ = 0;
  bool is_reading_ = false;
  uint8_t reading_state_ = 0; // 0, 1, 2, 2 就释放了，其实不会出现 2
  uint8_t reading_rob_id_ = 0;
  uint32_t reading_addr_ = 0;
  uint32_t reading_bytes_ = 0;
  uint32_t reading_type_ = 0; // 0 无符号， 1 有符号
  uint8_t value_ = 0;

  auto operator<=>(const WritingReadingState &) const = default;
};

struct MemoryState
{
  FetchToMemSign fetch_input_{};
  ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign flush_input_{};
  WritingReadingState writing_reading_state_{};

  auto operator<=>(const MemoryState &) const = default;
};

class Memory
{
public:
  Memory(const FetchToMemSign &fetch_input_sign,
         MemToFetchSign &fetch_output_sign, const LSQToMemSign &lsq_input_sign,
         MemToLSQSign &lsq_output_sign, MemToROBSign &rob_output_sign,
         const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
             &flush_input_sign);

  void LoadProgram(std::istream &input);
  void SetFetchReady(bool fetch_ready);
  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const FetchToMemSign &fetch_input_sign_;
  MemToFetchSign &fetch_output_sign_;
  const LSQToMemSign &lsq_input_sign_;
  MemToLSQSign &lsq_output_sign_;
  MemToROBSign &rob_output_sign_;
  const ROBToIssue_RAT_LSQ_RS_Decoder_CDB_Memory_ALUFlushSign
      &flush_input_sign_;
  uint8_t memory_[128 * 1024]{};
  MemoryState current_state_;
  MemoryState next_state_;
  bool fetch_ready_ = true;
};

#endif
