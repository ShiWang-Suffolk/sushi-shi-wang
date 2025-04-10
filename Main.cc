#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Sushi.hh"

Sushi my_shell; // Global Sushi instance

// Initialize the static constants
const size_t Sushi::MAX_INPUT = 256;
const size_t Sushi::HISTORY_LENGTH = 10;
const std::string Sushi::DEFAULT_PROMPT = "sushi> ";

int main(int argc, char *argv[])
{
  // DZ: This must be in the constructor
      // initialization
      Sushi::prevent_interruption();
      const char* home = std::getenv("HOME");
      if (!home) {
	//DZ: NOT an error!
	//std::cerr << "Error: $HOME not set.\n";
	//return EXIT_FAILURE;
      } else {
      std::string config_file = std::string(home) + "/sushi.conf";
      my_shell.read_config(config_file.c_str(), /*ok_if_missing=*/true);
      }
  
      //Processing
      // bool anyScript = false;
      for (int i = 1; i < argc; i++) {
          bool ok = my_shell.read_config(argv[i], /*ok_if_missing=*/false);
          if (!ok) {
	    // DZ: Already reported by read_config
	    //std::cerr << "Error reading script: " << argv[i] << std::endl;
              return EXIT_FAILURE;
          }
          // anyScript = true;
          if (my_shell.get_exit_flag()) {
              // Exit immediately
	    return /*0*/EXIT_SUCCESS;
          }
      }
  
      //IF haven't exited yet, enter interactive mode
      // DZ: Already checked previously
      // if (!my_shell.get_exit_flag()) {
          my_shell.mainloop(); 
	  //}
  
      return EXIT_SUCCESS;
}
