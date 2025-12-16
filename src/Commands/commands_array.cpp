#include "unified_commands.h"

// Create the commands array using the macro
const CommandStat commands[] = {
    #define CMD_ENTRY(name, func, param) COMMAND_ENTRY(name, func, param),
    INIT_COMMANDS(CMD_ENTRY)
    #undef CMD_ENTRY
};

// Define the number of commands
const int number_of_commands = sizeof(commands) / sizeof(commands[0]);