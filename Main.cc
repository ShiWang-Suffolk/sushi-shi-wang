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
    UNUSED(argc);
    UNUSED(argv);

  // New function call
  Sushi::prevent_interruption();
  
    const char* home = std::getenv("HOME");
    if (!home) {
        std::cerr << "Error: $HOME environment variable is not set." << std::endl;
        return EXIT_FAILURE;
    }

    std::string config_file = std::string(home) + "/sushi.conf";
    my_shell.read_config(config_file.c_str(), true); // ok_if_missing = true

    while (!my_shell.get_exit_flag()) {
        std::cout << Sushi::DEFAULT_PROMPT;
        std::string command = my_shell.read_line(std::cin);

        if (command.empty()) {
            continue; 
        }

 	// DZ: The parser will take care of this
	/*
       if (command == "exit") {
            my_shell.set_exit_flag(); 
            break;
        }

        if (command[0] == '!') {
            try {
                int history_index = std::stoi(command.substr(1));
                my_shell.re_parse(history_index); 
            } catch (const std::invalid_argument&) {
                std::cerr << "Error: Invalid history command." << std::endl;
            }
        } else {
	*/
            int parse_result = my_shell.parse_command(command);
            if (parse_result == 0) { 
                std::string line = "test command";
                my_shell.store_to_history(line);
            }
	    /*else { 
                std::cerr << "Error: Invalid command syntax." << std::endl;
		}
		}*/
    }

    // DZ: Unnecessary message
    // std::cout << "Exiting sushi shell." << std::endl;
    return EXIT_SUCCESS;
}
