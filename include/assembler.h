#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "FileRWUtils.h"
#include "AssertUtils.h"

#define PASS


const size_t MAX_INSTRUCT_LEN = 10;
const size_t LABELS_NUMBER    = 10;

enum AssemblerStatus_t {
    SUCCESS,
    FILE_NOT_FOUND,
    INVALID_ASM_CODE,
    MEMORY_ERROR,
    UNKNOWN_ERROR
};

struct Assembler_t {
    FileStat asm_file                = {};
    FileStat exe_file                = {};
    size_t   instruction_cnt         = 0;
    int*     byte_code               = NULL;
    int      labels[ LABELS_NUMBER ] = {};
};

enum ArgumentType {
    UNKNOWN  = -1,
    VOID     = 0,
    NUMBER   = 1,
    REGISTER = 2,
    LABEL    = 3
};

struct Argument {
    ArgumentType type = VOID;
    int value = 0;
};

void AssemblerCtor( Assembler_t* assembler, int argc, char** argv );
void AssemblerDtor( Assembler_t* assembler );

int AsmCodeToByteCode( Assembler_t* assembler );
int TranslateAsmToByteCode( Assembler_t* assembler, StrPar* strings );
int AsmCodeProcessing( char* instruction );
int RegisterNameProcessing( char* name );
int ArgumentProcessing( Argument* argument, const char* string );

AssemblerStatus_t AssemblerVerify( Assembler_t* assembler );
AssemblerStatus_t AssemblerDump( Assembler_t* assembler );

void OutputInFile(Assembler_t* assembler );

#endif //ASSEMBLER_H