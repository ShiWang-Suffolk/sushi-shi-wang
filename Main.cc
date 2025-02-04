#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Sushi.hh"

Sushi my_shell; // New global var

// Initialize the static constants
const size_t Sushi::MAX_INPUT = 256;
const size_t Sushi::HISTORY_LENGTH = 10;
const std::string Sushi::DEFAULT_PROMPT = "sushi> ";

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    // DZ: Moved to globals (not an error)
    // Sushi sushi;

    
    const char* home = std::getenv("HOME");
    // DZ: No need to exit because "ok if missing"
    if (!home) {
        std::cerr << "Error: $HOME environment variable is not set." << std::endl;
        return EXIT_FAILURE;
    }

    
    std::string config_file = std::string(home) + "/sushi.conf";
    // DZ: Must check return value
    my_shell.read_config(config_file.c_str(), true);

    while (true) {
        std::cout << Sushi::DEFAULT_PROMPT;
        std::string command = my_shell.read_line(std::cin);

        if (command == "exit") {
            break;
        }

	// DZ: It's store_to_history's job to check
        //if (!command.empty()) {
            my_shell.store_to_history(command);
            my_shell.show_history();
	    //}
    }

    return EXIT_SUCCESS;
}
