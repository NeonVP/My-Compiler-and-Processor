#ifndef SUPPORTED_COMMANDS_H
#define SUPPORTED_COMMANDS_H

#include "processor.h"
#include "Compiler.h"

const int number_of_commands = 20;

typedef enum {
    PUSH_CMD  = 1,
    POP_CMD   = 2,
    ADD_CMD   = 3,
    SUB_CMD   = 4,
    MUL_CMD   = 5,
    DIV_CMD   = 6,
    POW_CMD   = 7,
    SQRT_CMD  = 8,
    IN_CMD    = 9,
    OUT_CMD   = 10,
    JMP_CMD   = 11,
    JB_CMD    = 12,
    JA_CMD    = 13,
    JBE_CMD   = 14,
    JAE_CMD   = 15,
    JE_CMD    = 16,
    HLT_CMD   = 17,
    CALL_CMD  = 28,
    RET_CMD   = 29,
    LABEL_CMD  = 30,
    PUSHR_CMD = 33,
    POPR_CMD  = 34
} CommandCode_t;


#define COMMAND( cmd_name, cmd_func, cmd_type_of_param ) { cmd_name ##_CMD, #cmd_name, ON_PROC( cmd_func ,) cmd_type_of_param }

struct CommandStat {
    CommandCode_t command_number;
    const char*   command_name;
    #ifdef _PROC
        void      ( *function_pointer )( Processor_t* processor );
    #endif
    Argument_t    type_of_param;
};

extern const CommandStat commands[];

#endif