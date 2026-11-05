#include "parser.h"

int main() {
    chdir(getenv("HOME"));

    signal(SIGINT, SIG_IGN);   // prevent the shell from being interrupted by Ctrl+C
    signal(SIGTSTP, SIG_IGN);  // prevent the shell from being interrupted by Ctrl+Z
    signal(SIGTTIN, SIG_IGN);  // prevent the shell from being stopped by terminal input signals 
    signal(SIGTTOU, SIG_IGN);  // prevent the shell from being stopped by terminal output signals

    while(read_user_input()) {}

    return 0;
}