#include "../include/common.hpp"
#include "../include/utils.hpp"

void cmd_pwd() {
  std::cout << "\n\ncurrent path: " << fs::current_path() << "\n\n";
  pause();
  return;
}