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

void sigint_heredoc_handler(int sig) {
    // nothing to do here, just return to interrupt the read operation
}

int execute_command(Command* command) {
    if(is_buitlin(command->argv[0])) {
        return execute_builtin(command->argv);
    } else {
        //execute command

        int heredoc_fd[2];
        bool has_heredoc = (command->heredoc_delimiter != nullptr);

        // If there is a heredoc, the parent reads the input before forking
        if (has_heredoc) {
            if(pipe(heredoc_fd) == -1) {
                perror("myshell: pipe error");
                return -1;
            }

            struct sigaction sa;
            sa.sa_handler = sigint_heredoc_handler; // Set the signal handler for SIGINT
            sigemptyset(&sa.sa_mask); // Initialize the signal set to empty
            sa.sa_flags = 0; // No special flags
            sigaction(SIGINT, &sa, nullptr); // Set the new signal action for SIGINT

            while (true) {
                // Print the prompt first (just like Bash does for every line)
                printf("> ");
                fflush(stdout);

                
                char buffer[1024];
                if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
                    // If fgets returns NULL, it means EOF or an error occurred. We should handle this gracefully.
                    close(heredoc_fd[0]);
                    close(heredoc_fd[1]);
                    printf("\n");
                    return 0; // exit the heredoc input loop
                }
                printf("buffer: %s\n", buffer); // Debugging line to see what was read

                // Remove trailing newline, carriage return, spaces, and tabs
                int len = strlen(buffer);
                while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ' || buffer[len - 1] == '\t')) {
                    buffer[len - 1] = '\0';
                    len--;
                }

                // Check if it matches the delimiter immediately
                if (strcmp(buffer, command->heredoc_delimiter) == 0) {
                    break; // Exit right away, no extra prompt will be printed!
                }

                // Write to the pipe
                write(heredoc_fd[1], buffer, strlen(buffer));
                write(heredoc_fd[1], "\n", 1);
            }

            // Restore the default behavior for SIGINT after reading heredoc input
            signal(SIGINT, SIG_IGN);

            // Close the write end in the parent so the reader doesn't hang
            close(heredoc_fd[1]);
        }

        pid_t pid = fork();
        
        if(pid == 0) {
            if (command->output_filename != nullptr) {
                int fd;

                if(!command->append_output) {
                    fd = open(command->output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                } else {
                    fd = open(command->output_filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
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

            if (has_heredoc) {
                dup2(heredoc_fd[0], STDIN_FILENO);
                close(heredoc_fd[0]);
            }

            if (command->is_background) {
                // If the command is to be run in the background, we can ignore SIGINT
                signal(SIGINT, SIG_IGN);
            } else {
                // For foreground processes, restore the default behavior for SIGINT
                signal(SIGINT, SIG_DFL);
            }

            //child - execute command
            execvp(command->argv[0], command->argv);

            perror("myshell");
            exit(EXIT_FAILURE);
        } else if(pid > 0) {            
            if (command->is_background) {
                // Background process: print the PID and don't block waiting for it
                printf("[%d]\n", pid);
                fflush(stdout); // Ensure the output is printed immediately
            } else {
                // Foreground process: block and wait for completion
                int status;
                pid_t terminated_pid = waitpid(pid, &status, 0);

                if (WIFSIGNALED(status)) {
                    // Child was abnormally terminated by a signal
                    int signal_num = WTERMSIG(status);
                    printf("[Parent] Child %d was killed by signal: %d\n", terminated_pid, signal_num);
                    return 0;
                }
            }
        }
    }
    
    return 0;
}

int execute_pipeline(Pipeline* pipeline) {
    // if it is single command - just run it
    if (pipeline->command_count == 1) {
        if(pipeline->is_background) {
            pipeline->commands[0].is_background = true;
        }
        return execute_command(&pipeline->commands[0]);
    }

    int num_pipes = pipeline->command_count - 1;
    // all pipes' file descriptors
    int pipe_fds[2 * num_pipes];

    // create all pipes
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fds + i * 2) < 0) {
            perror("myshell: pipe error");
            return -1;
        }
    }

    pid_t pid;
    for (int i = 0; i < pipeline->command_count; ++i)
    {
        pid = fork();
        if (pid == 0) {
            // handle the input
            if (pipeline->commands[i].input_filename != nullptr) {
                // direction for input file 
                int fd_in = open(pipeline->commands[i].input_filename, O_RDONLY);
                if (fd_in < 0) {
                    perror("myshell: open input error");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            } else if (i > 0) {
                // if there isn't file - take last command as input
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }

            // handle output
            if (pipeline->commands[i].output_filename != nullptr) {
                int fd_out;
                if (!pipeline->commands[i].append_output) {
                    fd_out = open(pipeline->commands[i].output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                } else {
                    fd_out = open(pipeline->commands[i].output_filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
                if (fd_out < 0) {
                    perror("myshell: open output error");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            } else if (i < num_pipes) {
                // if there isn't file - take next command as output
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }

            if (pipeline->commands[i].is_background) {
                // If the command is to be run in the background, we can ignore SIGINT
                signal(SIGINT, SIG_IGN);
            } else {
                // For foreground processes, restore the default behavior for SIGINT
                signal(SIGINT, SIG_DFL);
            }

            // close all pipes' file descriptors
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fds[j]);
            }

            // run the commands
            execvp(pipeline->commands[i].argv[0], pipeline->commands[i].argv);
            perror("myshell: exec error");
            exit(EXIT_FAILURE);
        }
    }

    
    for (int j = 0; j < 2 * num_pipes; j++) {
        close(pipe_fds[j]);
    }

    if (pipeline->is_background) {
        // Background process: print the PID and don't block waiting for it
        printf("[%d]\n", pid);
        fflush(stdout); // Ensure the output is printed immediately
    } else {
        // wait for child proccess
        for (int j = 0; j < pipeline->command_count; j++) {
            int status;
            pid_t terminated_pid = waitpid(-1, &status, 0);

            if (WIFSIGNALED(status)) {
                // Child was abnormally terminated by a signal
                int signal_num = WTERMSIG(status);
                printf("[Parent] Child %d was killed by signal: %d\n", terminated_pid, signal_num);
                return 0;
            }
            
        }
    }

    return 0;
}

int execute_background_job(BackgroundJob *bg_job) {
    for (int i = 0; i < bg_job->pipeline_count; ++i)
    {
        Pipeline *pipeline = &bg_job->pipelines[i];
        if (pipeline->command_count <= 0) {
            continue;
        }

        int code = execute_pipeline(pipeline);
        if (code != 0) {
            return code;
        }
    }

    return 0;
}