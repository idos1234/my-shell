#include "parser.h"

void sigchld_handler(int sig) {
    // clear all terminated child processes to prevent zombie processes
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        // clear until there are no more terminated child processes
    }
}

int main() {
    chdir(getenv("HOME"));

    signal(SIGINT, SIG_IGN);   // prevent the shell from being interrupted by Ctrl+C
    signal(SIGTSTP, SIG_IGN);  // prevent the shell from being interrupted by Ctrl+Z
    signal(SIGTTIN, SIG_IGN);  // prevent the shell from being stopped by terminal input signals 
    signal(SIGTTOU, SIG_IGN);  // prevent the shell from being stopped by terminal output signals
    
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART; // to not interrupt system calls
    sigaction(SIGCHLD, &sa_chld, nullptr);

    while(read_user_input()) {}

    return 0;
}