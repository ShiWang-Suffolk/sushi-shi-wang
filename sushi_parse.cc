#include <cstring>
#include <cassert>
#include "Sushi.hh"

std::string *Sushi::unquote_and_dup(const char* s)
{
  assert(s);
  
  std::string result;
  result.reserve(std::strlen(s));
  
  while (*s) {
    if (*s == '\\') {
      ++s;
      if (!*s) {
        result += '\\';
        return new std::string(std::move(result));
      }
      
      switch (*s) {
      case 'a': result += '\a'; break;
      case 'b': result += '\b'; break;
      case 'e': result += '\x1B'; break;
      case 'f': result += '\f'; break;
      case 'n': result += '\n'; break;
      case 'r': result += '\r'; break;
      case 't': result += '\t'; break;
      case 'v': result += '\v'; break;
      case '\\': result += '\\'; break;
      case '\'': result += '\''; break;
      case '"': result += '"'; break;
      default:
	result += '\\';
	result += *s;
	break;
      }
      ++s;
    } else {
      result += *s++;
    }
  }
  return new std::string(std::move(result)); 
}

bool Sushi::re_execute() {
  if(!redo.empty()) {
    if (!parse_command(redo)) {
      store_to_history(redo);
    }
    redo = "";
    return true;
  }
  return false;
}

void Sushi::re_parse(int i) {
  size_t index = static_cast<size_t>(i);
  if (index == 0 || index > history.size()) {
    std::cerr << "!" << index << ": event not found" << std::endl;
  } else {
    redo = history[index - 1];
  }
}

//---------------------------------------------------------------
// Implement the function
std::string *Sushi::getenv(const char* s) 
{
  const char *value = std::getenv(s);
  return new std::string(value ? value : "");
}

// Implement the function
void Sushi::putenv(const std::string* name, const std::string* value)
{
  setenv(name->c_str(), value->c_str(), true);
  delete name;
  delete value;
}

//---------------------------------------------------------------
// Do not modify this function
void yyerror(const char* s) {
  std::cerr << "Parse error: " << s << std::endl;
}

