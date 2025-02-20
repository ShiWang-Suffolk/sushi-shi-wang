#include "Sushi.hh"
#include <fstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <string>


bool Sushi::get_exit_flag() const {
    return exit_flag;
}
void Sushi::set_exit_flag() {
    exit_flag = true;
}



std::string Sushi::read_line(std::istream& in) {
    std::string line;
    // DZ: Must check return value more explicitly
    if (!std::getline(in, line)) {
        if (in.fail() && !in.eof()) {
            std::perror("Error reading line");
        }
        return ""; 
    }

    if (line.length() > MAX_INPUT) {
        line = line.substr(0, MAX_INPUT);
        std::cerr << "Line too long, truncated to " 
                  << MAX_INPUT << " characters." << std::endl;
    }
    if (std::all_of(line.begin(), line.end(), [](char c){ return std::isspace(c); })) {
        return "";
    }

    return line;
}

void Sushi::store_to_history(std::string line) {
    if (line.empty()) return;

    if (history.size() == HISTORY_LENGTH) {
        history.pop_back();
    }
    history.insert(history.begin(), line);
}


bool Sushi::read_config(const char* fname, bool ok_if_missing) {
    std::ifstream file(fname);

    // DZ: Not always return false; 
    if (!file.is_open()) {
        if (!ok_if_missing) {
            // DZ: Wrong use of perror 
            std::cerr << "Error opening file: " << fname << std::endl;
        }
        return ok_if_missing;
    }

    // DZ: Wrong condition, the function breaks on the first blank line
    while (file.good()) {
        std::string line = read_line(file);

        if (!file.good() && !file.eof()) {
            // error
            std::perror("Error reading file");
            return false;
        }

        if (!line.empty()) {
            int ret = parse_command(line);
            if (ret != 0) {
                store_to_history(line);
            }
        }
        
    }

    // DZ: See above 

    file.close();
    if (file.bad()) {
        std::cerr << "Error occurred after closing file: " << fname << std::endl;
        return false;
    }

    return true;
}

void Sushi::show_history() const {
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << (i + 1) << " " << history[i] << std::endl;
    }
}

//---------------------------------------------------------
// New methods
int Sushi::spawn(Program *exe, bool bg)
{
  // Must be implemented
  UNUSED(exe);
  UNUSED(bg);

  return EXIT_SUCCESS;
}

void Sushi::prevent_interruption() {
  // Must be implemented
}

void Sushi::refuse_to_die(int signo) {
  // Must be implemented
  UNUSED(signo);
}

char* const* Program::vector2array() {
  // Must be implemented
  return nullptr; 
}

void Program::free_array(char *const argv[]) {
  // Must be implemented
  UNUSED(argv);
}

Program::~Program() {
  // Do not implement now
}
