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
    std::getline(in, line);

    if (in.fail() && !in.eof()) {
        std::perror("Error reading line");
        return "";
    }

    if (line.length() > MAX_INPUT) {
        std::cerr << "Line too long, truncated." << std::endl;
        return "";
    }

    if (line.empty() || std::all_of(line.begin(), line.end(), [](char c) { return std::isspace(c); })) {
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
            std::perror("Error opening file");
        }
        return false;
    }

    std::string line;
    while (true) {
        line = read_line(file);
        if (line.empty()) break;
        store_to_history(line);
    }

    if (file.bad()) {
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
