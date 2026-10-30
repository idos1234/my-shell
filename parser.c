#include "parser.h"

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

Command parse_command(char* str) {    
    // create command obj
    Command command = {0};

    // calculate argc
    command.argc = count_number_of_commands(str);

    // get all the argv
    command.argv = (char**)malloc((command.argc + 1) * sizeof(char*));
    command.argv[command.argc] = nullptr;
    
    if(!command.argv) {
        clear_command(&command);
        return command;
    }

    int start_index = 0;
    int end_index = 1;
    int command_index = 0;  
    
    while(str[end_index] != '\0') {
        if(str[end_index] == ' ') {
            // add new argument to argv
            command.argv[command_index] = (char*)malloc((end_index - start_index + 1) * sizeof(char));
            if(!command.argv[command_index]) {
                clear_command(&command);
                return command;
            }
            
            int len = end_index - start_index;

            // Copy the substring
            strncpy(command.argv[command_index], str + start_index, len);

            // Explicitly null-terminate the destination
            command.argv[command_index][len] = '\0'; 

            ++command_index;
            start_index = end_index + 1;
            end_index = start_index;
        }

        ++end_index;
    }

    if(command.argc == command_index + 1) {
        command.argv[command_index] = (char*)malloc((end_index - start_index + 1) * sizeof(char));
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

int check_for_redirect(Command* command) {
    for (int i = 0; i < command->argc; ++i)
    {
        if(!strcmp(command->argv[i], ">")) {
            //check if file exists
            if(i == (command->argc - 1)) {
                perror("redirect: no file found");
                return 0;
            }
            //clear previous file
            if(command->filename) {
                free(command->filename);
            }

            //set the new file
            command->filename = malloc((strlen(command->argv[i + 1]) + 1) * sizeof(char));
            if(!command->filename) {
                return -1;
            }
            strncpy(command->filename, command->argv[i+1], strlen(command->argv[i+1]));

            //remove > and filename from the argv
            command->argv[command->argc - 1] = nullptr;
            command->argv[command->argc - 2] = nullptr;
            command->argc = command->argc - 2;
            command->argv = realloc(command->argv, command->argc * sizeof(char*));
            if(!command->argv) {
                return -1;
            }
        }
    }

    return 0;
}




// Wait for commands
// true - for normal commands
// false - for interruption - stop shell
bool read_user_input() {
    //start the shell
    getcwd(dest, sizeof(dest));
    printf("myshell:%s$ ", dest);

    //get the input commands
    char input[MAX_INPUT];

    if (fgets(input, sizeof(input), stdin) == nullptr) {
        printf("Bye!\n");
        return false;
    }

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


    Command command = parse_command(input);
    
    if (command.argv == nullptr) {
        return false;
    }

    int code_redirect = check_for_redirect(&command); 
    if(code_redirect != 0) {
        return code_redirect;
    }

    int code = execute_command(&command);

    switch (code)
    {
    case 0:
        return true;
        break;
    case -1:
        return false;
        break;
    case 99:
        printf("Bye!\n");
        return false;
        break;
    default:
        break;
    }

    return true;
}


