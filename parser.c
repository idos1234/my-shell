#include "parser.h"

struct termios orig_termios;


void remove_extra_spaces(char *str) {
    int i = 0; // Pointer to read the original string
    int j = 0; // Pointer to write the cleaned string

    // 1. Skip leading spaces
    while (str[i] == ' ') {
        i++;
    }

    // 2. Process characters and intermediate spaces
    while (str[i] != '\0') {
        if(str[i] == '\t') {
            str[i] = ' ';
        }

        if (str[i] != ' ') {
            // Copy non-space characters
            str[j++] = str[i++];
        } else {
            // When a space is hit, write exactly one space
            str[j++] = ' ';
            // Skip all subsequent consecutive spaces
            while (str[i] == ' ' || str[i] == '\t') {
                i++;
            }
        }
    }

    // 3. Remove trailing space if it exists
    if (j > 0 && str[j - 1] == ' ') {
        str[j - 1] = '\0';
    } else {
        str[j] = '\0'; // Null-terminate the string
    }
}

int count_number_of_commands(char* str) {
    int i = 0;
    int count = 0;

    while(str[i] != '\0') {
        if(str[i] == ' ') {
            ++count;
        }

        ++i;
    }

    return count+1;
}

int count_number_of_pipelines(char* str) {
    int count = 0;
    int i = 0;

    while(str[i] != '\0') {
        if(str[i] == '|') {
            ++count;
        }

        ++i;
    }

    return count;
    
}

int count_number_of_background_jobs(char* str) {
    int count = 0;
    int i = 0;

    while(str[i] != '\0') {
        if(str[i] == '&') {
            ++count;
        }

        ++i;
    }

    return count;
    
}

int check_for_redirect(Command* command) {
    for (int i = 0; i < command->argc; ++i)
    {
        // check for output redirection
        if(!strcmp(command->argv[i], ">")) {
            //check if file exists
            if(i == (command->argc - 1)) {
                perror("redirect: no file found");
                return 0;
            }
            //clear previous file
            if(command->output_filename) {
                free(command->output_filename);
            }

            //set the new file
            command->output_filename = strdup(command->argv[i + 1]);
            if(!command->output_filename) {
                return -1;
            }

            //set append as false
            command->append_output = false;

            //remove > and filename from the argv
            for (int j = i; j <= command->argc - 2; ++j) {
                command->argv[j] = command->argv[j + 2];
            }
            
            command->argc = command->argc - 2;
            command->argv = realloc(command->argv, (command->argc + 1) * sizeof(char*));
            if(!command->argv) {
                return -1;
            }
            command->argv[command->argc] = nullptr;

            if(i >= command->argc) {
                break;
            }
            --i;
        }

        // check for output redirection
        else if(!strcmp(command->argv[i], "<")) {
            //check if file exists
            if(i == (command->argc - 1)) {
                perror("redirect: no file found");
                return 0;
            }
            //clear previous file
            if(command->input_filename) {
                free(command->input_filename);
            }

            //set the new file
            command->input_filename = strdup(command->argv[i + 1]);
            if(!command->input_filename) {
                return -1;
            }


            //remove > and filename from the argv
            for (int j = i; j <= command->argc - 2; ++j) {
                command->argv[j] = command->argv[j + 2];
            }

            command->argc = command->argc - 2;
            command->argv = realloc(command->argv, (command->argc + 1) * sizeof(char*));
            if(!command->argv) {
                return -1;
            }
            command->argv[command->argc] = nullptr;

            if(i >= command->argc) {
                break;
            }
            --i;
        }

        // check for output - append redirection
        else if(!strcmp(command->argv[i], ">>")) {
            //check if file exists
            if(i == (command->argc - 1)) {
                perror("redirect: no file found");
                return 0;
            }
            //clear previous file
            if(command->output_filename) {
                free(command->output_filename);
            }

            //set the new file
            command->output_filename = strdup(command->argv[i + 1]);
            if(!command->output_filename) {
                return -1;
            }

            //set append as true
            command->append_output = true;

            //remove > and filename from the argv
            for (int j = i; j <= command->argc - 2; ++j) {
                command->argv[j] = command->argv[j + 2];
            }
            
            command->argc = command->argc - 2;
            command->argv = realloc(command->argv, (command->argc + 1) * sizeof(char*));
            if(!command->argv) {
                return -1;
            }
            command->argv[command->argc] = nullptr;

            if(i >= command->argc) {
                break;
            }
            --i;
        }

        // check for input - heredoc redirection
        else if(!strcmp(command->argv[i], "<<")) {
            if (i == (command->argc - 1)) {
                perror("redirect: no delimiter found");
                return 0;
            }

            // Clear previous delimiter if exists
            if (command->heredoc_delimiter) {
                free(command->heredoc_delimiter);
            }

            // Set the new delimiter and clean any hidden \r, \n, or spaces
            command->heredoc_delimiter = strdup(command->argv[i + 1]);
            if (!command->heredoc_delimiter) {
                return -1;
            }

            int delim_len = strlen(command->heredoc_delimiter);
            while (delim_len > 0 && (command->heredoc_delimiter[delim_len - 1] == '\n' || 
                                     command->heredoc_delimiter[delim_len - 1] == '\r' || 
                                     command->heredoc_delimiter[delim_len - 1] == ' ' || 
                                     command->heredoc_delimiter[delim_len - 1] == '\t')) {
                command->heredoc_delimiter[delim_len - 1] = '\0';
                delim_len--;
            }

            // Remove << and the delimiter from argv
            for (int j = i; j <= command->argc - 2; ++j) {
                command->argv[j] = command->argv[j + 2];
            }

            command->argc = command->argc - 2;
            command->argv = realloc(command->argv, (command->argc + 1) * sizeof(char*));
            if (!command->argv) {
                return -1;
            }
            command->argv[command->argc] = nullptr;

            if (i >= command->argc) {
                break;
            }
            --i;
        }

        // check for background execution
        else if(!strcmp(command->argv[i], "&")) {
            // Set the background execution flag
            command->is_background = true;

            //remove & from the argv
            for (int j = i; j <= command->argc - 1; ++j) {
                command->argv[j] = command->argv[j + 1];
            }
            
            command->argc = command->argc - 1;
            command->argv = realloc(command->argv, (command->argc + 1) * sizeof(char*));
            if(!command->argv) {
                return -1;
            }
            command->argv[command->argc] = nullptr;

            break; // No need to check further since & should be at the end
        }
    }

    return 0;
}

int check_for_environment_variables(Command* command) {
    for (int i = 0; i < command->argc; ++i) {
        if (command->argv[i][0] == '$') {
            char* env_var_name = command->argv[i] + 1; // Skip the '$'
            char* env_var_value = getenv(env_var_name);
            if (env_var_value) {
                // Replace the argument with the environment variable's value
                free(command->argv[i]); // Free the old string
                command->argv[i] = strdup(env_var_value); // Duplicate the new value
                if (!command->argv[i]) {
                    return -1; // Memory allocation failed
                }
            } else {
                // If the environment variable doesn't exist, replace it with an empty string
                free(command->argv[i]);
                command->argv[i] = strdup(""); // Set to empty string
                if (!command->argv[i]) {
                    return -1; // Memory allocation failed
                }
            }
        }
    }
    return 0;
}

Command parse_command(char* str) {    
    // create command obj
    Command command = {0};

    // calculate argc
    command.argc = count_number_of_commands(str);

    // get all the argv
    command.argv = (char**)malloc((command.argc + 1) * sizeof(char*));
    if(!command.argv) {
        clear_command(&command);
        return command;
    }
    command.argv[command.argc] = nullptr;

    int start_index = 0;
    int end_index = 0;
    int command_index = 0;  
    
    while(str[end_index] != '\0') {
        if(str[end_index] == ' ') {
            int len = end_index - start_index;
            if (len > 0) {
                command.argv[command_index] = (char*)malloc(len + 1);
                if(!command.argv[command_index]) {
                    clear_command(&command);
                    return command;
                }
                strncpy(command.argv[command_index], str + start_index, len);
                command.argv[command_index][len] = '\0'; 
                ++command_index;
            }
            start_index = end_index + 1;
        }

        ++end_index;
    }

    //handle the last word in the line
    int len = end_index - start_index;
    if (len > 0 && command_index < command.argc) {
        command.argv[command_index] = (char*)malloc((len + 1) * sizeof(char));
        if(!command.argv[command_index]) {
            clear_command(&command);
            return command;
        }
        
        int len = end_index - start_index;

        // Copy the substring
        strncpy(command.argv[command_index], str + start_index, len);

        // Explicitly null-terminate the destination
        command.argv[command_index][len] = '\0';
    }
    

    return command;
}

Pipeline parse_pipeline(char *str, bool is_background) {
    // create pipeline obj
    Pipeline pipeline = {0};
    if(is_background) {
        pipeline.is_background = true;
    }

    // calculate how many commands there are
    pipeline.command_count = count_number_of_pipelines(str) + 1;

    // get all the commands array
    pipeline.commands = (Command*)malloc((pipeline.command_count + 1) * sizeof(Command));
    if(!pipeline.commands) {
        clear_pipeline(&pipeline);
        return pipeline;
    }

    int start_index = 0;
    int cmd_idx = 0;
    int i = 0;

    while (str[i] != '\0') {
        if (str[i] == '|') {
            // get sub string until |
            int len = i - start_index;
            char *sub_cmd_str = (char*)malloc(len + 1);
            strncpy(sub_cmd_str, str + start_index, len);
            sub_cmd_str[len] = '\0';

            //remove extra spaces - around the pipe line
            remove_extra_spaces(sub_cmd_str);
            
            // parse the current command
            pipeline.commands[cmd_idx] = parse_command(sub_cmd_str);
            if(check_for_environment_variables(&pipeline.commands[cmd_idx]) != 0) {
                clear_pipeline(&pipeline);
                return pipeline;
            }
            if(is_background) {
                pipeline.commands[cmd_idx].is_background = true;
            }

            int code_redirect = check_for_redirect(&pipeline.commands[cmd_idx]); // check for redirects
            if(code_redirect != 0) {
                clear_pipeline(&pipeline);
                return pipeline;
            }
            
            free(sub_cmd_str);
            cmd_idx++;
            start_index = i + 1;
        }
        i++;
    }

    // handle the last command
    int len = i - start_index;
    char *sub_cmd_str = (char*)malloc(len + 1);
    strncpy(sub_cmd_str, str + start_index, len);
    sub_cmd_str[len] = '\0';
    
    remove_extra_spaces(sub_cmd_str);
    pipeline.commands[cmd_idx] = parse_command(sub_cmd_str);
    if(check_for_environment_variables(&pipeline.commands[cmd_idx]) != 0) {
        clear_pipeline(&pipeline);
        return pipeline;
    }

    check_for_redirect(&pipeline.commands[cmd_idx]);
    
    free(sub_cmd_str);

    return pipeline;
}

BackgroundJob parse_background_job(char *str) {
    // create background job obj
    BackgroundJob bg_job = {0};

    // calculate how many background jobs there are
    bg_job.pipeline_count = count_number_of_background_jobs(str);
    if(bg_job.pipeline_count == 0) {
        bg_job.pipeline_count = count_number_of_pipelines(str) + 1; // At least one job if no '&' is found
    }

    // get all the background job arrays
    bg_job.pipelines = (Pipeline*)malloc((bg_job.pipeline_count + 1) * sizeof(Pipeline));
    if(!bg_job.pipelines) {
        clear_background_job(&bg_job);
        return bg_job;
    }

    int start_index = 0;
    int background_job_idx = 0;
    int i = 0;

    while (str[i] != '\0') {
        if (str[i] == '&') {
            // get sub string until &
            int len = i - start_index - 1;
            char *sub_cmd_str = (char*)malloc(len + 1);
            strncpy(sub_cmd_str, str + start_index, len);
            sub_cmd_str[len] = '\0';

            //remove extra spaces - around the background job
            remove_extra_spaces(sub_cmd_str);

            if (strlen(sub_cmd_str) == 0) {
                free(sub_cmd_str);
                continue;
            }

            // parse the current background job
            bg_job.pipelines[background_job_idx].is_background = true;
            bg_job.pipelines[background_job_idx] = parse_pipeline(sub_cmd_str, true);

            free(sub_cmd_str);
            ++background_job_idx;
            start_index = i + 1;
        }
        i++;
    }

    if(i > start_index) {
        // handle the last background job
        int len = i - start_index;
        char *sub_cmd_str = (char*)malloc(len + 1);
        strncpy(sub_cmd_str, str + start_index, len);
        sub_cmd_str[len] = '\0';

        remove_extra_spaces(sub_cmd_str);
        bg_job.pipelines[background_job_idx] = parse_pipeline(sub_cmd_str, false);
        free(sub_cmd_str);
    }
    
    return bg_job;
}


// Restores the terminal to its default "cooked" state
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Configures the terminal into raw mode
void enableRawMode() {
    // Get current terminal attributes
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    // Register clean-up function to restore settings on exit
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    /*
     * Input Flags (c_iflag):
     * BRKINT: Turn off break condition triggering SIGINT
     * ICRNL: Stop translating Carriage Return (\r) to Newline (\n)
     * INPCK: Disable parity checking
     * ISTRIP: Disable stripping the 8th bit of characters
     * IXON: Disable software flow control (Ctrl-S / Ctrl-Q)
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /*
     * Output Flags (c_oflag):
     * OPOST: Turn off all output processing (like converting \n to \r\n)
     */
    raw.c_oflag &= ~(OPOST);

    /*
     * Control Flags (c_cflag):
     * CS8: Set character size to 8 bits per byte
     */
    raw.c_cflag |= (CS8);

    /*
     * Local Flags (c_lflag):
     * ECHO: Turn off echoing of typed characters
     * ICANON: Turn off canonical (line-by-line) mode
     * IEXTEN: Disable extended input processing (Ctrl-V)
     * ISIG: Disable signal characters (Ctrl-C to SIGINT, Ctrl-Z to SIGTSTP)
     */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    /*
     * Control Characters (c_cc):
     * VMIN: Minimum number of bytes needed before read() can return
     * VTIME: Maximum time to wait before read() returns (in tenths of a second)
     */
    raw.c_cc[VMIN] = 1;  // Read returns as soon as 1 byte is available
    raw.c_cc[VTIME] = 0; // No timeout

    // Apply the changes immediately using TCSAFLUSH
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

// Wait for commands
// true - for normal commands
// false - for interruption - stop shell
bool read_user_input() {
    //start the shell
    getcwd(dest, sizeof(dest));
    printf("myshell:%s$ ", dest);
    fflush(stdout);

    // Enable raw mode for better input handling
    enableRawMode();

    //get the input commands
    char input[MAX_INPUT];

    int index = 0;
    while (1) {
        // Read character directly from raw stdin file descriptor
        if (read(STDIN_FILENO, &input[index], 1) == -1) {
            perror("read\n");
            printf("Bye!\n");
            fflush(stdout);
            fflush(stderr);
            return false;
        }

        printf("%c", input[index]); // Echo the character back to the terminal
        fflush(stdout);

        if (input[index] == '\r') {
            printf("\n");
            fflush(stdout);
            disableRawMode(); // Restore terminal settings before processing the input
            break;
        }
        ++index;
    }
    
    // Replace any carriage return characters with newline characters to standardize the input
    input[strcspn(input, "\r")] = '\n';


    // Check if the input was too long and didn't fit into MAX_INPUT
    if (strchr(input, '\n') == NULL) {
        // Clear remaining data from stdin buffer until newline or EOF
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    } else {
        // Remove the '\n' safely
        input[strcspn(input, "\n")] = '\0';
    }


    // remove the extra spaces
    remove_extra_spaces(input);
    
    // Check if the command line is empty (press Enter only)
    if (input[0] == '\0') {
        return true;
    }


    BackgroundJob bg_job = parse_background_job(input);
    
    if (bg_job.pipelines == nullptr) {
        return false;
    }


    int code = execute_background_job(&bg_job);

    switch (code)
    {
    case 0:
        return true;
    case -1:
        return false;
    case 99:
        printf("Bye!\n");
        return false;
    }

    return true;
}


