#include "Sushi.hh"
#include <cstring>


/*
typedef struct yy_buffer_state* YY_BUFFER_STATE;

extern "C" {
    typedef struct yy_buffer_state* YY_BUFFER_STATE;
    int yyparse();
    int yylex_destroy();
    YY_BUFFER_STATE yy_scan_string(const char *str);
    void yy_delete_buffer(YY_BUFFER_STATE);
}
*/

// DZ: Implemented in the wrong file
// DZ: You implemented only 2 characters out of 11.
// `...` is not an implementation
std::string* Sushi::unquote_and_dup(const char* s) {
    std::string result;
  
    for (const char *p = s; *p != '\0'; ++p) {
        if (*p == '\\') {
            ++p;
            if (*p == '\0') {
                break;
            }
            switch (*p) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                // ...
                default:
                    result += '\\';
                    result += *p;
                    break;
            }
        } else {
            result += *p;
        }
    }
    return new std::string(result);
  }

// DZ: Implemented in the wrong file
  void Sushi::re_parse(int i) {
    //    if (i <= 0 || i > (int)history.size()) {
    // DZ: C++ way:
      if (i <= 0 || i > static_cast<int>(history.size())) {
        std::cerr << "Error: !" << i << ": event not found\n";
        return;
    }
    std::string cmd = history[i - 1];
    int ret = parse_command(cmd);
    // DZ: All unnecessary
    /*
    if (ret == 1) {  
        std::cout << cmd << std::endl;  
    } else {  
        std::cerr << "Error: Invalid command syntax in history item " << i << std::endl;
    }
    */
    // DZ: Must store the command in history
    if (ret) {
      // ...
    }
}

//---------------------------------------------------------------
// Implement the function
std::string *Sushi::getenv(const char* s) 
{
  if (!s) {
    return new std::string("");
}

const char* val = std::getenv(s);
if (val == nullptr) {
    return new std::string("");
}
return new std::string(val);
}

void Sushi::putenv(const std::string* name, const std::string* value)
{
  if (!name || !value) {
    // If the pointer is invalid, return directly
    return;
  }
std::string envLine = *name + "=" + *value;
// DZ: strdup() creates memory leaks
//char* cenv = ::strdup(envLine.c_str());
 char* cenv = const_cast<char*>(envLine.c_str());
::putenv(cenv);
}

//---------------------------------------------------------------
// Do not modify this function
void yyerror(const char* s) {
  std::cerr << "Parse error: " << s << std::endl;
}

