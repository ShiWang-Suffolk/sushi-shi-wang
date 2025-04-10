#include "Sushi.hh"
#include <fstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <string>

#include <unistd.h>     
#include <sys/types.h>  
#include <sys/wait.h>   
#include <csignal>      
#include <signal.h>




bool Sushi::get_exit_flag() const {
    return exit_flag;
}
void Sushi::set_exit_flag() {
    exit_flag = true;
}

void Sushi::store_to_history(std::string line) {
    if (line.empty()) return;

    if (history.size() == HISTORY_LENGTH) {
        history.pop_back();
    }
    history.insert(history.begin(), line);
}


std::string Sushi::read_line(std::istream& in) {
    std::string line;
    if (!std::getline(in, line)) {
        if (!in.eof() && in.fail()) {
            std::perror("Error reading line");
            in.clear(); // Clear the error flag to avoid continuous error in the flow state
        }
        // When reading fails or EOF is reached, an empty string is returned.
        return "";
    }

    if (line.length() > MAX_INPUT) {
        line = line.substr(0, MAX_INPUT);
        std::cerr << "Line too long, truncated to "
                  << MAX_INPUT << " characters." << std::endl;
    }

    if (std::all_of(line.begin(), line.end(), [](char c){ return std::isspace(static_cast<unsigned char>(c)); })) {
        return "";
    }

    return line;
}

bool Sushi::read_config(const char* fname, bool ok_if_missing) {
    std::ifstream file(fname);
    if (!file.is_open()) {
        if (!ok_if_missing) {
            std::perror(("Error opening file: " + std::string(fname)).c_str());
        }
        return ok_if_missing;
    }

    // Avoid premature exit after blank line or error
    while (true) {
        std::string line = read_line(file);
        if (!file) {
            if (!file.eof() && file.fail()) {
                std::perror("Error reading file");
                file.close();
                return false;
            }
            // Encountering EOF or an irrecoverable error, exit the loop
            break;
        }
        if (!line.empty()) {
            int ret = parse_command(line);
            if (ret != 0) {
                store_to_history(line);
            }
        }
    }


    file.close();
    if (file.bad()) {
        std::cerr << "Error occurred after closing file: " << fname << std::endl;
        return false;
    }

    return true;
}

void Sushi::show_history() const {
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << (i + 1) << " " << history[i] << std::endl;
    }
}

//---------------------------------------------------------
// New methods
int Sushi::spawn(Program *exe, bool bg)
{
    pid_t pid = fork();
    if (pid < 0) {
        std::perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // subprocess
        char *const* argv = exe->vector2array();
        if (!argv) {
            std::perror("vector2array returned null");
            _exit(127); 
        }
        execvp(argv[0], argv);
        // if execvp fail
        std::perror(argv[0]); 
        exe->free_array(argv);
	// DZ: Wrong exit value
        _exit(EXIT_FAILURE/*127*/);
    } else {
        // parent process
        if (!bg) {
           
            int status = 0;
            if (waitpid(pid, &status, 0) < 0) {
                std::perror("waitpid");
                return EXIT_FAILURE;
            }
            int code = 0;
            if (WIFEXITED(status)) {
                code = WEXITSTATUS(status); 
            } else if (WIFSIGNALED(status)) {
                code = 128 + WTERMSIG(status);
            }
	    // DZ: Cannot use putenv for `?`, must use setenv
            Sushi::putenv(
                new std::string("?"), 
                new std::string(std::to_string(code))
            );
	    // DZ: no
            // return code;

            
        }
    }
    // DZ: Missing return value
    return EXIT_SUCCESS;
}

void Sushi::prevent_interruption() {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler =/*Sushi::*/refuse_to_die;
    sa.sa_flags=SA_RESTART;           // Avoid system calls interrupted by signals returning errors

    if (sigaction(SIGINT, &sa, nullptr) < 0) {
        std::perror("sigaction");
    }
}

void Sushi::refuse_to_die(int signo) {
    if (signo==SIGINT) {
        std::cerr << "Type exit to exit the shell" << std::endl; // To prevent the prompt from appearing repeatedly due to an error state
        std::cin.clear(); 
    }
}

void Sushi::mainloop() {
        while (!get_exit_flag()) {
	  // DZ: The default prompt is DEFAULT_PROMPT
	  // std::string ps1_default = "sushi> ";
            std::string *ps1_val = Sushi::getenv("PS1");
            std::string ps1 = ps1_val ? *ps1_val : "";
            delete ps1_val;
            if (ps1.empty()) {
	      ps1 = DEFAULT_PROMPT/*ps1_default*/;
            }
    
            // Output Prompt
            std::cout << ps1;
            std::cout.flush();
            std::string line = read_line(std::cin);
            // If the read fails or EOF is reached, exit the loop
            if (!std::cin) {
                break;
            }
            if (line.empty()) {
                continue;
            }
            int ret = parse_command(line);
            if (ret != 0) {
                store_to_history(line);
            }
        }
        // std::cout << "Shell exiting...\n";
}

char* const* Program::vector2array() {
    if (args==nullptr || args->empty()) {
        return nullptr;
    }
    size_t count=args->size();
    char** argv=new(std::nothrow) char*[count + 1];
    if (!argv) {
        // Failed to allocate memory
        return nullptr;
    }

    for (size_t i = 0; i < count; i++) {
      // DZ: This piece of code is unnecessary because of `char* CONST*`
      // DZ: Do not copy without necessity
        std::string* s = args->at(i); // strdup dynamically allocates and copies the string content, which needs to be freed later
        argv[i] = ::strdup(s->c_str());
        if (!argv[i]) {
            for (size_t j = 0; j < i; j++) {
                ::free(argv[j]);
            }
            delete[] argv;
            return nullptr;
        }
    }
    argv[count] = nullptr;
    return argv;
}

void Program::free_array(char *const argv[]) {
    if (!argv) return;
    // DZ: See above
    for (size_t i = 0; argv[i] != nullptr; i++) {
        ::free(argv[i]);
    }
    delete[] argv;
}

Program::~Program() {
  // Do not implement now
}
