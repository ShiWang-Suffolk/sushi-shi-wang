#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Sushi.hh"

// Initialize the static constants
const size_t Sushi::MAX_INPUT = 256;
const size_t Sushi::HISTORY_LENGTH = 10;
const std::string Sushi::DEFAULT_PROMPT = "sushi> ";

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    Sushi sushi;

    
    const char* home = std::getenv("HOME");
    if (!home) {
        std::cerr << "Error: $HOME environment variable is not set." << std::endl;
        return EXIT_FAILURE;
    }

    
    std::string config_file = std::string(home) + "/sushi.conf";
    sushi.read_config(config_file.c_str(), true);

    while (true) {
        std::cout << Sushi::DEFAULT_PROMPT;
        std::string command = sushi.read_line(std::cin);

        if (command == "exit") {
            break;
        }

        if (!command.empty()) {
            sushi.store_to_history(command);
            sushi.show_history();
        }
    }

    return EXIT_SUCCESS;
}
