/*
 * Name: Muhammad El Sayed Muhammad Mustafa Awad
 * ID: 6149
 *
 * IMP: I tested 'firefox &' command, and it works fine but when I close firefox
 *      of the time I get this message I don't know why. I did some research, and
 *      I found that it is something related to firefox and not to the program. It
 *      also happens when I use the terminal. If you ignore the message and type any
 *      command the program will continue normally.
 *
 * Message: ###!!! [Child][RunMessage] Error: Channel closing: too late to send/recv, messages will be lost
 *
 */

#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <sys/wait.h>
#include <csignal>

char** create_command(char *command);
void sigchldHandler(int sig);

int main() {

    // Opening and closing the log.txt to make sure it's empty for every run
    // by opening it for writing.
    std::ofstream file {"log.txt", std::ios_base::out};
    file.close();

    while (true) {

        // Getting the current path, so we can see which directory we are in when using 'cd'
        std::string current_path = std::filesystem::current_path();
        std::cout << "Muhammad:" << current_path << " root$ ";

        // Getting the command from the user.
        std::string single_line_command;
        std::getline(std::cin, single_line_command);

        // True when the command ends with &, so the parent will not wait its child.
        bool continue_execution {false};

        if (single_line_command[single_line_command.size() - 1] == '&') {
            single_line_command[single_line_command.size() - 1] = '\0';
            continue_execution = true;
        }

        char *char_command = const_cast<char *>(single_line_command.c_str());

        // Break down the command at each space, so we can pass it as array to execvp().
        char **command = create_command(char_command);

        // Check if the user when to exit.
        if (!strcmp(command[0], "exit")) {
            std::cout << "[Process completed]" << std::endl;
            break;

            // Change directory
        } else if (!strcmp(command[0], "cd")) {

            // To pass the new_path in a correct way to chdir().
            std::string new_path = current_path + '/' + command[1];

            // chdir accepts const char *
            if (chdir(new_path.c_str()) < 0) {
                std::cout << command[1] << " " << strerror(errno) << std::endl;
            }

            // To prevent the creation of a new child.
            continue;
        }

        // Signals handler for the death of a child.
        struct sigaction sa {};
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = sigchldHandler;

        sigaction(SIGCHLD, &sa, nullptr);

        pid_t pid = fork();

        if (pid < 0) {
            std::cout << "Can't fork process: " << strerror(errno) << std::endl;
        }
        if (!pid) {

            // execvp() returns -1 for failure.
            if (execvp(command[0], command) == -1) {
                std::cout << command[0] << ": command not found" << std::endl;
                exit(errno);
            }
        } else {
            int pid_status;

            // 'WNOHANG' macro is used to prevent the parent for waiting the child to complete.
            // waitpid() error is not handle because of a child recently died without
            // having, the parent, to wait for it to die.
            waitpid(pid, &pid_status, continue_execution ? WNOHANG : 0);
        }

        // Free the space allocated throw create_command().
        free(command);
    }

    return EXIT_SUCCESS;
}

char** create_command(char *command) {

    // Returns first token
    char *token = strtok(command, " ");

    unsigned counter {0};

    char** real_command = static_cast<char **>(malloc(sizeof(token)));
    real_command[counter++] = token;

    // Keep doing tokens while one of the delimiters present in command.
    while (token != nullptr) {
        token = strtok(nullptr, " ");
        real_command = static_cast<char **>(realloc(real_command, sizeof(token)));
        real_command[counter++] = token;
    }
    return real_command;
}

void sigchldHandler(int sig) {
    int status;
    pid_t childPid;

    // Open log file for appending.
    std::ofstream file {"log.txt", std::ios_base::app};

    // It will check if any zombie-children exist.
    // If yes, one of them is reaped and its exit status returned.
    while((childPid = waitpid(-1, &status, WNOHANG)) > 0);

    if (file.is_open()) {
        file << "Child process was terminated" << std::endl;
    } else {
        exit(EXIT_FAILURE);
    }

    file.close();
}
