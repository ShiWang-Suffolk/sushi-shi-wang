#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include <csignal>
#include <cassert>
#include <sys/wait.h>
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
    if(!parse_command(line)) {
      store_to_history(line);
    }
  }
  
  return true; 
}

void Sushi::store_to_history(std::string line)
{
  // Do not insert empty lines
  if (line.empty()) {
    return;    
  }

  // Is the history buffer full?
  while (history.size() >= HISTORY_LENGTH) {
    history.pop_front();
  }
  
  history.emplace_back(line);
}

void Sushi::show_history() 
{
  int index = 1;

  // `history` itself will be inserted
  if (history.size() == HISTORY_LENGTH) {
    history.pop_front();
  }
  
  for (const auto &cmd: history) {
    std::cout << std::setw(5) << index++ << "  " << cmd << std::endl;
  }
  
  // `history` itself will be inserted
  std::cout << std::setw(5) << index++ << "  " << "history" << std::endl;
}

void Sushi::set_exit_flag()
{
  exit_flag = true;
}

bool Sushi::get_exit_flag() const
{
  return exit_flag;
}

int Sushi::spawn(Program *exe, bool bg)
{
  UNUSED(bg);
  
  pid_t pid = fork();

  if (pid == -1) { // Failed to fork
    std::perror("fork");
    return EXIT_FAILURE;
  }

  if (pid == 0) { // Child    
    char* const* args = exe->vector2array(); // No need to delete this array!
    assert(args);
    
    execvp(args[0], args);
    std::perror(args[0]);
    // Do not run atexit handlers and flush buffers
    _exit(EXIT_FAILURE);
  }

  // Parent
  int status;
  if(waitpid(pid, &status, 0) != pid) {
    std::perror("waitpid");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void Sushi::prevent_interruption() {
  struct sigaction sa;
  sa.sa_handler = refuse_to_die;
  // Restart the read() system call
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGINT, &sa, nullptr) != 0) {
    std::perror("sigaction");
    std::exit(EXIT_FAILURE);
  }
}

void Sushi::refuse_to_die(int signo) {
  UNUSED(signo);
  std::cerr << "Type exit to exit the shell" << '\n';
}

void Sushi::mainloop() {
  // Must be implemented
}

char* const* Program::vector2array() {
  // std::vector<std::string*> *args -> char *const argv[]
  assert(args);
  
  size_t size = args->size();
  char** array = new char*[size + 1]; // Allocate an array of char*
  
  for (size_t i = 0; i < size; ++i) {
    assert((*args)[i]);
    array[i] = const_cast<char*>((*args)[i]->c_str()); // Copy string content
  }
  
  array[size] = nullptr; // Null-terminate the array
  return array;
}

Program::~Program() {
  // Do not implement now
}
