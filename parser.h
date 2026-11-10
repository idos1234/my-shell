#ifndef PARSER_H

#define PARSER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "executor.h"
#include "command_struct.h"
#include <termios.h>
    

#define MAX_INPUT 1024

bool read_user_input();

#endif