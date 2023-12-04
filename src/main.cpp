#include <iostream>
#include <chm/fact.h>

unsigned int factorial(unsigned int number) {
  return number <= 1 ? number : factorial(number - 1) * number;
}

int main() {
  std::cout << "Hello World!\n";
  return 0;
}
