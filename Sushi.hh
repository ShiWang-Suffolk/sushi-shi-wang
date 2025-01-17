#pragma once
#include <string> 

class Sushi {
private:
  int history; // `int` is a placehoder; replace it with a type of your choice
  static const size_t HISTORY_LENGTH;
  static const size_t MAX_INPUT;

public:
  Sushi() : history() {};
  std::string read_line(std::istream &in);
  bool read_config(const char *fname, bool ok_if_missing);
  void store_to_history(std::string line);
  void show_history();

  static const std::string DEFAULT_PROMPT;
};

#define UNUSED(expr) do {(void)(expr);} while (0)
