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

char** parse_command(char* command) {    
    // calculate number of commands
    int number_of_commands = count_number_of_commands(command);

    // get all the commands
    char** commands = (char**)malloc(number_of_commands * sizeof(char*));
    if(!commands) {
        return nullptr;
    }

    int start_index = 0;
    int end_index = 1;
    int command_index = 0;
    
    while(command[end_index] != '\0') {
        if(command[end_index] == ' ') {
            // add new command
            commands[command_index] = (char*)malloc((end_index - start_index + 1) * sizeof(char));
            if(!commands[command_index]) {
                return nullptr;
            }
            
            int len = end_index - start_index;

            // Copy the substring
            strncpy(commands[command_index], command + start_index, len);

            // Explicitly null-terminate the destination
            commands[command_index][len] = '\0'; 

            ++command_index;
            start_index = end_index + 1;
            end_index = start_index;
        }

        ++end_index;
    }

    if(number_of_commands == command_index + 1) {
        commands[command_index] = (char*)malloc((end_index - start_index + 1) * sizeof(char));
        if(!commands[command_index]) {
            return nullptr;
        }
        
        int len = end_index - start_index;

        // Copy the substring
        strncpy(commands[command_index], command + start_index, len);

        // Explicitly null-terminate the destination
        commands[command_index][len] = '\0';
    }
    


    return commands;
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



    char** commands = parse_command(input);
    
    if (commands == nullptr) {
        return false;
    }

    int code = execute_command(commands);

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


