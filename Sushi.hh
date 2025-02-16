#pragma once
#include <iostream> // New include
#include <string>
#include <vector>

class Sushi {
private:
    std::vector<std::string> history;
    static const size_t HISTORY_LENGTH;
    static const size_t MAX_INPUT;
  bool exit_flag = false; // New attribute

public:
    Sushi() : history() {};
     std::string read_line(std::istream &in);
     static std::string* unquote_and_dup(const char* s); // New method
  static std::string *getenv(const char *name); // New method
    bool read_config(const char *fname, bool ok_if_missing);
    void store_to_history(std::string line);
    void show_history() const;

  void re_parse(int i); // New method
  void set_exit_flag(); // New method
  bool get_exit_flag() const; // New method
  static int parse_command(const std::string &command);
  static const std::string DEFAULT_PROMPT;
};

#define UNUSED(expr) do {(void)(expr);} while (0)
// New declaration
extern Sushi my_shell;