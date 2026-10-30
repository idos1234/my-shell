#include "command_struct.h"

void clear_command(Command* command) {
    for (int i = 0; i < command->argc; ++i)
    {
        free(command->argv[i]);
    }

    if(command->argv) {
        free(command->argv);
    }
    if(command->filename) {
        free(command->filename);
    }
    command->filename = nullptr;
    command->argv = nullptr;
    command->argc = 0;
    
}