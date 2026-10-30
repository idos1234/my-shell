#ifndef COMMAND_STRUCT_H
#define COMMAND_STRUCT_H

#include <stdlib.h>

typedef struct {
    char **argv;
    int argc;
    char* filename;
} Command;

void clear_command(Command* command);

#endif