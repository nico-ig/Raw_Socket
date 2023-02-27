#include "./headers/cores.h"
#include <iostream>

int main() {
  std::cout << BLACK << "Black\n";
  std::cout << RED << "Red\n";
  std::cout << GREEN << "Green\n";
  std::cout << YELLOW << "Yellow\n";
  std::cout << BLUE << "Blue\n";
  std::cout << MAGENTA << "Magenta\n";
  std::cout << CYAN << "Cyan\n";
  std::cout << WHITE << "White\n";
  std::cout << BOLDBLACK << "Bold Black\n";
  std::cout << BOLDRED << "Bold Red\n";
  std::cout << BOLDGREEN << "Bold Green\n";
  std::cout << BOLDYELLOW << "Bold Yellow\n";
  std::cout << BOLDBLUE << "Bold Blue\n";
  std::cout << BOLDMAGENTA << "Bold Magenta\n";
  std::cout << BOLDCYAN << "Bold Cyan\n";
  std::cout << BOLDWHITE << "Bold White\n";
  std::cout << RESET << "Default\n";

  return 0;
}