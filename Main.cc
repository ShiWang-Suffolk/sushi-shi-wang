#include <cstdlib>
#include <iostream>
#include "Sushi.hh"

// Initialize the static constants
const size_t Sushi::MAX_INPUT = 256;
const size_t Sushi::HISTORY_LENGTH = 10;
const std::string Sushi::DEFAULT_PROMPT = "sushi> ";

int main(int argc, char *argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  // A placeholder
  std::cout << Sushi::DEFAULT_PROMPT << "Hello, world!" << std::endl;
  
  return EXIT_SUCCESS;
}
