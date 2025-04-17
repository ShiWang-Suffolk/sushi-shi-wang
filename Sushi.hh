#pragma once
#include <iostream>
#include <deque>
#include <vector> 

/// I/O redirections, as in "foobar < foo > bar"
class Redirection {
private:
  // stdin, stdout-write, stdout-append
  const std::string *redir_in, *redir_out1, *redir_out2;
  
public:
  void set_out1(std::string *fname) { redir_out1 = fname; }
  void set_out2(std::string *fname) { redir_out2 = fname; }
  void set_in(std::string *fname)   { redir_in = fname; }
  void set_in(Redirection &redir)   {
    redir_in = redir.redir_out1 ? redir.redir_out1 : redir.redir_out2;
  }
};

// The program to be executed
class Program {
private:
  std::vector<std::string*> *args; // Arguments, including the program name
  Redirection redir;
  // The previous program in the pipeline, if any; NULL otherwise
  Program *pipe = nullptr;

public:
  Program(std::vector<std::string*> *args) : args(args) {};
  ~Program();
  void set_pipe(Program *pipe) { this->pipe = pipe; };
  void set_redir(Redirection &redir) { this->redir = redir; };
  
  // Helper method(s)
  // Converts the args to whatever `execvp` expects
  char* const* vector2array();
  Program *prev() { return pipe; };
};

class Pipe {
private:
  Program *head, *tail;
public:
  Pipe(Program *p) : head(p), tail(p) {};
  Program *hd() { return head; }
  Program *tl() { return tail; }
  void tl(Program *p) { tail = p; }
};

class Sushi {
private:
  std::deque<std::string> history; 
  static const size_t HISTORY_LENGTH = 10;
  static const size_t MAX_INPUT_SIZE = 256;
  bool exit_flag = false; 
  std::string redo;

public:
  Sushi();
  static std::string read_line(std::istream &in);
  static std::string *unquote_and_dup(const char *s); 
  static std::string *getenv(const char *name);
  static void putenv(const std::string *name, const std::string *value);
  bool read_config(const char *fname, bool ok_if_missing);
  void store_to_history(std::string line);
  void show_history();
  void re_parse(int i); 
  bool re_execute();
  void set_exit_flag(); 
  bool get_exit_flag() const; 
  static int parse_command(const std::string command);
  void mainloop(); 
  void pwd(); // New method
  void cd(std::string *new_dir); // New method
  int spawn(Program *exe, bool bg) const;   
  static void prevent_interruption(); 
  static void refuse_to_die(int signo);
    
  static const std::string DEFAULT_PROMPT;
  static const std::string DEFAULT_CONFIG;
};

template<typename T> void UNUSED(T&&) {}

extern Sushi my_shell;


