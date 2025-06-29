#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <cstring>
#include <cerrno>

bool running = true;

void signalHandler(int signal_num) {
    if (signal_num == SIGINT) {
        std::string response;
        std::cout << "\nDo you really want to quit (y/n)? ";
        std::cin >> response;
        if (response == "y") {
            running = false;
        }
    }
}

void executeCommand(std::vector<std::string>& args, bool inBackground) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::vector<char*> cargs;
        for (auto& arg : args) cargs.push_back(&arg[0]);
        cargs.push_back(nullptr);

        execvp(cargs[0], cargs.data());
        // Only reached if exec fails
        std::cerr << "minishell: command not found: " << args[0] << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        if (inBackground) {
            std::cout << "[" << pid << "]" << std::endl;
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    } else {
        std::cerr << "Failed to fork." << std::endl;
    }
}

void processLine(const std::string& line) {
    std::istringstream iss(line);
    std::string segment;
    std::vector<std::string> args;

    while (iss >> segment) {
        args.push_back(segment);
    }

    if (args.empty()) return;

    bool inBackground = false;
    if (args.back() == "&") {
        inBackground = true;
        args.pop_back();
    }

    if (args[0] == "exit") {
        std::cout << "Time elapsed: 0h 0m 0s" << std::endl; // Simplified time display
        running = false;
    } else if (args[0] == "cd") {
        if (args.size() > 1) {
            if (chdir(args[1].c_str()) != 0) {
                std::cerr << "minishell: cd: " << strerror(errno) << std::endl;
            }
        } else {
            std::cerr << "minishell: cd: missing argument" << std::endl;
        }
    } else {
        executeCommand(args, inBackground);
    }
}

int main() {
    std::string input;
    char* cwd;
    char buff[1024];
    signal(SIGINT, signalHandler);

    std::clock_t start = std::clock();

    while (running) {
        cwd = getcwd(buff, 1024);
        std::cout << (cwd ? std::string(cwd) : "unknown directory") << " > ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            processLine(input);
        }
    }

    double elapsed_time = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "Time elapsed: " << int(elapsed_time / 3600) << "h " 
              << int((elapsed_time / 60)) % 60 << "m " 
              << int(elapsed_time) % 60 << "s" << std::endl;

    return 0;
}
