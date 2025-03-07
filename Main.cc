#include <cstdlib>
#include "Sushi.hh"

// Initialize the static constants
Sushi my_shell; 
const std::string Sushi::DEFAULT_PROMPT = "sushi> ";
const std::string Sushi::DEFAULT_CONFIG = "sushi.conf";

int main(int argc, char *argv[])
{
  // Use argc and argv!
  
  // Move this into the constructor
  //-------------------------------------------
  Sushi::prevent_interruption();
  
  const char *home_dir = std::getenv("HOME");
  // OK if missing!
  if (home_dir) {
    std::string config_path = std::string(home_dir) + "/" + Sushi::DEFAULT_CONFIG;
    my_shell.read_config(config_path.c_str(), true);
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // Move this into the main loop method
  //-------------------------------------------
  while(!my_shell.get_exit_flag()) {
    std::cout << Sushi::DEFAULT_PROMPT;
    std::string command = Sushi::read_line(std::cin);
    if(!Sushi::parse_command(command)) {
      // Re-execute from history if needed
      if(!my_shell.re_execute()) {
	// Do not insert the bangs (!)
	my_shell.store_to_history(command);
      }
    }
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // my_shell.mainloop();
  
  return EXIT_SUCCESS;
}
