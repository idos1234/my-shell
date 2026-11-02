#include "command_struct.h"

void clear_command(Command* command) {
    for (int i = 0; i < command->argc; ++i)
    {
        free(command->argv[i]);
    }

    if(command->argv) {
        free(command->argv);
    }
    if(command->output_filename) {
        free(command->output_filename);
    }
    if(command->input_filename) {
        free(command->input_filename);
    }
    command->input_filename = nullptr;
    command->output_filename = nullptr;
    command->argv = nullptr;
    command->argc = 0;
    
}

void clear_pipeline(Pipeline* pipeline) {
    for (int i = 0; i < pipeline->command_count; ++i)
    {
        clear_command(&pipeline->commands[i]);
    }

    if(pipeline->commands) {
        free(pipeline->commands);
    }

    pipeline->commands = nullptr;
    pipeline->command_count = 0;
}