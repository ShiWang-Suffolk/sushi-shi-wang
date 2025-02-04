#include "Sushi.hh"
#include <fstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cctype>

std::string Sushi::read_line(std::istream& in) {
    std::string line;
    // DZ: Must check return value
    std::getline(in, line);

    if (in.fail() && !in.eof()) {
        std::perror("Error reading line");
        return "";
    }

    if (line.length() > MAX_INPUT) {
        std::cerr << "Line too long, truncated." << std::endl;
	// DZ: How is it truncated?
        return "";
    }

    // DZ: The first condition is redundant
    if (/*line.empty() ||*/ std::all_of(line.begin(), line.end(), [](char c) { return std::isspace(c); })) {
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

    if (!file.is_open()) {
        if (!ok_if_missing) {
	  // DZ: Wrong use of perror	  
	  // std::perror("Error opening file");
	  std::perror(fname);
        }
	// DZ: Not always
        return false;
    }

    std::string line;
    while (true) {
        line = read_line(file);
        if (line.empty()) {
	  // DZ: Wrong condition, the function breaks on the first blank line
	  break;
	}
        store_to_history(line);
    }

    if (file.bad()) {
      // DZ: See above
        std::perror("Error closing file");
        return false;
    }

    return true;
}

void Sushi::show_history() {
    int count = 1;
    for (const auto& entry : history) {
        std::cout << std::setw(5) << std::right << count++ << "  " << entry << std::endl;
    }
}

void Sushi::set_exit_flag()
{
  // To be implemented
}

bool Sushi::get_exit_flag() const
{
  return false; // To be fixed
}
