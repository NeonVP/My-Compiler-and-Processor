#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "FileRWUtils.h"

#define PASS

#ifdef _ASM
#define ON_ASM(...) __VA_ARGS__
#else
#define ON_ASM(...)
#endif

const size_t MAX_INSTRUCT_LEN = 10;
const size_t LABELS_NUMBER    = 10;

// enum AssemblerStatus_t {
//     SUCCESS,
//     FILE_NOT_FOUND,
//     INVALID_ASM_CODE,
//     MEMORY_ERROR,
//     UNKNOWN_ERROR
// };

enum Argument_t {
    UNKNOWN  = -1,
    VOID     = 0,
    NUMBER   = 1,
    REGISTER = 2,
    LABEL    = 3
};

struct ArgumentStat {
    Argument_t type;
    int        value;
};

struct LabelStat {
    char* label_name;
    int   instruction_ptr;
};

struct Assembler_t {
    FileStat asm_file;
    FileStat exe_file;
    size_t   instruction_cnt;
    int*     byte_code;
    LabelStat* labels;
};

static const char* register_names[] = { 
    "ROX", "RAX", "RBX", "RCX", "RDX", 
    "REX", "RFX", "RGX", "RHX", "RIX"
};

void AssemblerCtor( Assembler_t* assembler, int argc, char** argv );
void AssemblerDtor( Assembler_t* assembler );

int AsmCodeToByteCode( Assembler_t* assembler );
int TranslateAsmToByteCode( Assembler_t* assembler, StrPar* strings );
int AsmCodeProcessing( char* instruction );
int RegisterNameProcessing( char* name );
int ArgumentProcessing( Assembler_t* assembler, ArgumentStat* argument, const char* string );

// AssemblerStatus_t AssemblerVerify( Assembler_t* assembler );
// AssemblerStatus_t AssemblerDump( Assembler_t* assembler );

void OutputInFile(Assembler_t* assembler );

#endif //ASSEMBLER_H
