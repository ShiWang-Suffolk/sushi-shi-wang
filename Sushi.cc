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

#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
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
            in.clear();
        }
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

    while (true) {
        std::string line = read_line(file);
        if (!file) {
            if (!file.eof() && file.fail()) {
                std::perror("Error reading file");
                file.close();
                return false;
            }
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

    // Collect all programs in the pipeline
    std::vector<Program*> pipeline;
    for (Program* p = exe; p != nullptr; p = p->pipe) {
        pipeline.push_back(p);
    }
    std::reverse(pipeline.begin(), pipeline.end());

    std::vector<pid_t> child_pids;
    int in_fd = STDIN_FILENO;
    int fd[2] = { -1, -1 };

    // Iterate over each command
    for (size_t i = 0; i < pipeline.size(); ++i) {
        Program* p = pipeline[i];

        // If not last command, create a new pipe
        if (i + 1 < pipeline.size()) {
            if (pipe(fd) < 0) {
                perror("pipe");
                return EXIT_FAILURE;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            // Child process

            // Extract redirection info
            auto &R    = p->get_redir();
            auto *rin  = R.get_in();
            auto *rout = R.get_out1();
            auto *rapp = R.get_out2();

            // stdin: file redirection or pipe
            if (rin) {
                int fdin = open(rin->c_str(), O_RDONLY);
                if (fdin < 0) { perror(rin->c_str()); _exit(EXIT_FAILURE); }
                dup2(fdin, STDIN_FILENO);
                close(fdin);
            } else if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // stdout: file redirection, pipe, or default stdout
            if (rout) {
                int fdout = open(rout->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fdout < 0) { perror(rout->c_str()); _exit(EXIT_FAILURE); }
                dup2(fdout, STDOUT_FILENO);
                close(fdout);
            } else if (rapp) {
                int fdout = open(rapp->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fdout < 0) { perror(rapp->c_str()); _exit(EXIT_FAILURE); }
                dup2(fdout, STDOUT_FILENO);
                close(fdout);
            } else if (i + 1 < pipeline.size()) {
                dup2(fd[1], STDOUT_FILENO);
            }

            // Close pipe file descriptors
            if (fd[0] >= 0) close(fd[0]);
            if (fd[1] >= 0) close(fd[1]);

            // Execute command
            char* const* argv = p->vector2array();
            if (!argv) {
                perror("vector2array");
                _exit(127);
            }
            execvp(argv[0], argv);
            perror(argv[0]);
            p->free_array(argv);
            _exit(EXIT_FAILURE);
        } else {
            // Parent process
            child_pids.push_back(pid);
            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            // Prepare in_fd for next command
            if (i + 1 < pipeline.size()) {
                close(fd[1]);
                in_fd = fd[0];
            }
        }
    }

    // Wait for all children if not background job
    if (!bg) {
        for (pid_t cpid : child_pids) {
            int status;
            waitpid(cpid, &status, 0);
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

// Two new methods to implement
void Sushi::pwd() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
    } else {
        perror("getcwd");
    }
}
// Change directory
void Sushi::cd(std::string *new_dir) {
    if (!new_dir) return;
    if (chdir(new_dir->c_str()) != 0) {
        // Print error message, e.g., "cd: /some/path: reason"
        std::string msg = "cd: " + *new_dir;
        perror(msg.c_str());
    }
    delete new_dir;
}

// Convert vector of args to argv array
char* const* Program::vector2array() {
    if (!args || args->empty()) return nullptr;
    size_t count = args->size();// Failed to allocate memory
    char** argv = new(std::nothrow) char*[count + 1];
    if (!argv) return nullptr;
    for (size_t i = 0; i < count; ++i) {
        argv[i] = ::strdup(args->at(i)->c_str());// strdup dynamically allocates and copies the string content, which needs to be freed later
        if (!argv[i]) {
            for (size_t j = 0; j < i; ++j) ::free(argv[j]);
            delete[] argv;
            return nullptr;
        }
    }
    argv[count] = nullptr;
    return argv;
}
// Free argv array
void Program::free_array(char *const argv[]) {
    if (!argv) return;
    for (size_t i = 0; argv[i] != nullptr; ++i) {
        ::free(argv[i]);
    }
    delete[] argv;
}
// Destructor
Program::~Program() {
    // Do not implement now
}
