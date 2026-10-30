#ifndef COMMAND_STRUCT_H
#define COMMAND_STRUCT_H

#include <stdlib.h>

typedef struct {
    char **argv;
    int argc;
    char* output_filename;
    char* input_filename;
} Command;

void clear_command(Command* command);

#endif