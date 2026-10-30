#include "executor.h"

char dest[256];
const char* builting_commands[NUMBER_OF_BUILTIN] = {"cd", "quit"};

bool is_buitlin(char* command) {
    for (int i = 0; i < NUMBER_OF_BUILTIN; ++i)
    {
        if(!strcmp(command, builting_commands[i])) {
            return true;
        }
    }

    return false;
    
}

int execute_cd(char ** argv) {
    if(argv[1] != nullptr && argv[2] != nullptr) {
        return -2;
    }

    if(argv[1] == nullptr || !strcmp(argv[1], "~")) {
        return chdir(getenv("HOME"));
    }
    return chdir(argv[1]);
}

int execute_quit() {
    return 99;
}

int execute_builtin(char **argv) {
    if(!strcmp(argv[0], "cd")) {
        int code = execute_cd(argv);
        if(code == -1) {
            printf("Path not found\n\n");
            return 0;
        } else if(code == -2) {
            printf("cd: too many argument\n\n");
        }
        return code;
    } else if(!strcmp(argv[0], "quit")) {
        return execute_quit();
    }

    return -1;
}


int execute_command(Command* command) {
    if(is_buitlin(command->argv[0])) {
        return execute_builtin(command->argv);
    } else {
        //execute command

        pid_t pid = fork();
        
        if(pid == 0) {
            if (command->output_filename != nullptr) {
                int fd = open(command->output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("myshell: open error");
                    exit(EXIT_FAILURE);
                }
                // set stdout - the file
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (command->input_filename != nullptr) {
                int fd = open(command->input_filename, O_RDONLY, 0644);
                if (fd < 0) {
                    perror("myshell: open error");
                    exit(EXIT_FAILURE);
                }
                // set stdout - the file
                dup2(fd, STDIN_FILENO);
                close(fd);
            }


            //child - execute command
            execvp(command->argv[0], command->argv);

            perror("myshell");
            exit(EXIT_FAILURE);
        } else if(pid > 0) {
            int status;
            pid_t terminated_pid = waitpid(pid, &status, 0);

            if (WIFSIGNALED(status)) {
                // Child was abnormally terminated by a signal
                int signal_num = WTERMSIG(status);
                printf("[Parent] Child %d was killed by signal: %d\n", terminated_pid, signal_num);
                return -1;
            }
        }
    }
    
    return 0;
}