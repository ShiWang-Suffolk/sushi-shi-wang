#include "Sushi.hh"

std::string *Sushi::unquote_and_dup(const char* s)
{
  if (!s) { // Not required, but it is safer this way
    return nullptr;
  }
  
  std::string result;
  while (*s) {
    if (*s == '\\') {
      if (*(s + 1)) {
	switch (*(s + 1)) {
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
	  result += *s;
	  result += *(s + 1);
	  break;
	}
	s += 2; // Skip escape sequence
      } else { // Dangling \ at the EOS; undefined behavior
	result += '\\';
	return new std::string(result); 
      }
    } else {
      result += *s;
      ++s;
    }
  }
  return new std::string(result); 
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
// Do not modify this function YET
std::string *Sushi::getenv(const char* s) 
{
  return new std::string(s); // Must be changed - eventually
}

//---------------------------------------------------------------
// Do not modify this function
void yyerror(const char* s) {
  std::cerr << "Parse error: " << s << std::endl;
}

