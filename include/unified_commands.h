#ifndef UNIFIED_COMMANDS_H
#define UNIFIED_COMMANDS_H

#include "Compiler.h"

// Define ON_PROC macro similar to ON_ASM
#ifdef _PROC
#define ON_PROC(...) __VA_ARGS__
#else
#define ON_PROC(...)
#endif

// Forward declaration to avoid circular dependency
struct Processor_t;

// Define the macro for operations
#define INIT_COMMANDS(macros) \
    macros(PUSH, ProcPush, NUMBER) \
    macros(POP, ProcPop, VOID) \
    macros(ADD, ProcAdd, VOID) \
    macros(SUB, ProcSub, VOID) \
    macros(MUL, ProcMul, VOID) \
    macros(DIV, ProcDiv, VOID) \
    macros(POW, ProcPow, VOID) \
    macros(SQRT, ProcSqrt, VOID) \
    macros(IN, ProcIn, VOID) \
    macros(OUT, ProcOut, VOID) \
    macros(PUSHR, ProcPushR, REGISTER) \
    macros(POPR, ProcPopR, REGISTER) \
    macros(CALL, ProcCall, LABEL) \
    macros(RET, ProcRet, VOID) \
    macros(JMP, ProcJump, LABEL) \
    macros(JE, ProcJump, LABEL) \
    macros(JB, ProcJump, LABEL) \
    macros(JA, ProcJump, LABEL) \
    macros(JBE, ProcJump, LABEL) \
    macros(JAE, ProcJump, LABEL) \
    macros(HLT, ProcHlt, VOID) \
    macros(MARK, ProcMark, LABEL) \
    macros(LABEL, ProcLabel, VOID)

// Enum for command codes
enum CommandCode {
    #define DEF_CMD(name, func, param) name##_CMD,
    INIT_COMMANDS(DEF_CMD)
    #undef DEF_CMD
};

// Structure for command statistics
struct CommandStat {
    CommandCode command_number;
    const char* command_name;
    #ifdef _PROC
        void (*function_pointer)(Processor_t* processor);
    #endif
    Argument_t type_of_param;
};

// Declare the commands array
extern const CommandStat commands[];

// Define helper macro to create command entries
#define COMMAND_ENTRY(cmd_name, cmd_func, cmd_param) { \
    cmd_name##_CMD, \
    #cmd_name, \
    ON_PROC(cmd_func,) \
    cmd_param \
}

#endif // UNIFIED_COMMANDS_H