#include "Sushi.hh"

// Initialize the static constants
const std::string Sushi::DEFAULT_PROMPT = "sushi> ";
const std::string Sushi::DEFAULT_CONFIG = "sushi.conf";

// The constructor will read the configuration file, if present
Sushi my_shell; 

int main(int argc, char *argv[])
{
  // Execute command-line scripts
  for (int i = 1; i < argc; ++i) {
    if(!my_shell.read_config(argv[i], false /* NOT ok if missing */)) {
      return EXIT_FAILURE;
    }
  }
  
  // Enter the main loop for good
  my_shell.mainloop();
  
  return EXIT_SUCCESS;
}
