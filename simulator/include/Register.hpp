#ifndef REGISTER_HPP
#define REGISTER_HPP

#include "Sign.hpp"

struct RegisterState
{
  uint32_t registers_[32]{};

  auto operator<=>(const RegisterState &) const = default;
};

class Register
{
public:
  explicit Register(const ROBToRegisterSign &rob_input_sign);

  uint32_t Read(uint8_t register_id);

  void UpdateCurrent();
  void Execute();
  void UpdateNext();

private:
  const ROBToRegisterSign &rob_input_sign_;
  RegisterState current_state_;
  RegisterState next_state_;
};

#endif
