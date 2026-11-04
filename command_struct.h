#ifndef COMMAND_STRUCT_H
#define COMMAND_STRUCT_H

#include <stdlib.h>

typedef struct {
    char **argv;
    int argc;
    char* output_filename;
    char* input_filename;
    bool append_output;
    char* heredoc_delimiter;
    bool is_background;
} Command;

typedef struct Pipeline {
    Command *commands;
    int command_count;
} Pipeline;

void clear_command(Command* command);
void clear_pipeline(Pipeline* pipeline);

#endif