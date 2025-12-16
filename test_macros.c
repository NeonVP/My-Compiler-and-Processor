#include "include/unified_commands.h"

// Define a macro to print out the command names
#define PRINT_CMD(name, func, param) #name "_CMD ",
const char* command_names[] = {
    INIT_COMMANDS(PRINT_CMD)
};

#undef PRINT_CMD