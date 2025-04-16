#include "Sushi.hh"
#include <fstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>

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
int Sushi::spawn(Program *exe, bool bg) {
    if (!exe) return EXIT_FAILURE;

    // Build a vector containing the pipeline commands in execution order.
    std::vector<Program*> pipeline;
    for (Program* p = exe; p != nullptr; p = p->pipe) {
        pipeline.push_back(p);
    }
    std::reverse(pipeline.begin(), pipeline.end());

    std::vector<pid_t> child_pids;
    int in_fd = STDIN_FILENO;  // Initial input for the first command is standard input
    int fd[2] = { -1, -1 };
    // Define a helper lambda to get the command name from a Program instance.
    auto getCmdName = [](Program* prog) -> std::string {
        std::string cmdName;
        char* const* arr = prog->vector2array();
        if (arr && arr[0]) {
            cmdName = arr[0];
        }
        prog->free_array(arr);
        return cmdName;
    };

    // Iterate over the pipeline in execution order
    for (size_t i = 0; i < pipeline.size(); i++) {
        Program* p = pipeline[i];
        std::string cmdName = getCmdName(p);
        
        // For all but the last command, create a pipe
        if (i < pipeline.size() - 1) {
            if (pipe(fd) < 0) {
                perror("pipe");
                return EXIT_FAILURE;
            }
            //Use this to detect how many pipes were created, for now I can only do one. Maybe I'll improve it later
            fprintf(stderr, "Created pipe: read end = %d, write end = %d\n", fd[0], fd[1]);
        } else {
            // For the last command, no new pipe is needed.
            fd[1] = STDOUT_FILENO; 
        }

        

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (pid == 0) { // Child process
            // Redirect standard input
            if (in_fd != STDIN_FILENO) {
                if (dup2(in_fd, STDIN_FILENO) < 0) {
                    perror("dup2 for stdin");
                    _exit(1);
                }
                close(in_fd);
            }
            if (i < pipeline.size() - 1) {
                if (dup2(fd[1], STDOUT_FILENO) < 0) {
                    perror("dup2 for stdout");
                    _exit(1);
                }
                close(fd[1]);
                close(fd[0]); // Close read end in child; not needed here
            }
            
            char *const* argv = p->vector2array();
            if (!argv) {
                perror("vector2array returned null");
                _exit(127);
            }
            execvp(argv[0], argv);
            perror(argv[0]); // if execvp fails
            p->free_array(argv);
            _exit(EXIT_FAILURE);
        } else { // Parent process
            
            child_pids.push_back(pid);

            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            // For all but the last command, the next command's input will come from the read end of the pipe
            if (i < pipeline.size() - 1) {
                close(fd[1]);
                in_fd = fd[0];
                fprintf(stderr, "Parent: Forked: Set in_fd to fd %d for next command\n", in_fd);
            }
        }
    }

    // Wait for all child processes 
    if (!bg) {
        for (pid_t pid : child_pids) {
            int status = 0;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                return EXIT_FAILURE;
            }
            
        }
    }
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
    for (size_t i = 0; argv[i] != nullptr; i++) {
        ::free(argv[i]);
    }
    delete[] argv;
}

Program::~Program() {
  // Do not implement now
}
