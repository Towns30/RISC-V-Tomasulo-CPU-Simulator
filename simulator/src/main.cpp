#include "CPU.hpp"

#include <iostream>

int main()
{
  CPU cpu;
  cpu.LoadProgram(std::cin);
  int clock = 0;
  while (!cpu.Halted())
  {
    cpu.Cycle();
    clock++;
  }
  std::cout << cpu.Result() << '\n';
  std::cout << clock << '\n';
  return 0;
}
