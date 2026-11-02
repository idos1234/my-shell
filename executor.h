#ifndef EXECUTOR_H

#define EXECUTOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "command_struct.h"

#define NUMBER_OF_BUILTIN 2

extern char dest[256];
int execute_pipeline(Pipeline *pipeline);

#endif