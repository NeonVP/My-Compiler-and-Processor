#ifndef SUPPORTED_COMMANDS_H
#define SUPPORTED_COMMANDS_H

#include "unified_commands.h"

// For backward compatibility
typedef CommandCode CommandCode_t;

// Declare extern variable for command count (commands array is already declared in unified_commands.h)
extern const int number_of_commands;

// Also define as macro for direct use
#define NUMBER_OF_COMMANDS_MACRO (0 INIT_COMMANDS(COUNT_COMMANDS))

#endif