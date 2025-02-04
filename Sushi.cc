#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include "Sushi.hh"

std::string Sushi::read_line(std::istream &in)
{
  std::string line;
  if(!std::getline (in, line)) {// Has the operation failed?
    if(!in.eof()) { 
      std::perror("getline");
    }
    return "";
  }
    
  // Is the line empty?
  if(std::all_of(line.begin(), line.end(), isspace)) {
    return "";
  }

  // Is the line too long?
  if(line.size() > MAX_INPUT_SIZE) {
    line.resize(MAX_INPUT_SIZE);
    std::cerr << "Line too long, truncated." << std::endl;
  }
  
  return line; 
}

bool Sushi::read_config(const char *fname, bool ok_if_missing)
{
  // Try to open a config file
  std::ifstream config_file(fname);
  if (!config_file) {
    if (!ok_if_missing) {
      std::perror(fname);
      return false;
    }
    return true;
  }

  // Read the config file
  while(!config_file.eof()) {
    std::string line = read_line(config_file);
    store_to_history(line);
  }
  
  return true; 
}

void Sushi::store_to_history(std::string line)
{
  if (line.empty()) {
    return;    
  }

  // Is the history buffer full?
  while (history.size() >= HISTORY_LENGTH) {
    history.pop_front();
  }
  
  history.emplace_back(line);
}

void Sushi::show_history() const
{
  int index = 1;
  for (const auto &cmd: history) {
    std::cout << std::setw(5) << index++ << "  " << cmd << std::endl;
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
