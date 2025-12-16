#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "FileRWUtils.h"
#include "stack.h"

const int REGS_NUMBER = 8;

#ifdef _PROC
#define ON_PROC(...) __VA_ARGS__
#else
#define ON_PROC(...)
#endif

enum ProcessorStatus_t {
    SUCCESS,
    FILE_NOT_FOUND,
    INVALID_EXE_CODE,
    UNKNOWN_ERROR
};

struct Processor_t {
    Stack_t     stk;    
    Stack_t     refund_stk;
    int*        byte_code;
    size_t      instruction_ptr;
    size_t      instruction_count;
    StackData_t regs[ REGS_NUMBER ];
};

// Include unified commands after Processor_t is defined
#include "unified_commands.h"

void ProcCtor( Processor_t* processor, size_t stack_size, size_t refund_stack_size );
void ProcDtor( Processor_t* processor );


// ProcessorStatus_t ProcVerify( Processor_t* processor );
ProcessorStatus_t ProcDump( const Processor_t* processor, const int error );

void PrintByteCodeInline  ( const Processor_t* processor );
void PrintRegisters       ( const Processor_t* processor );


void ExeFileToByteCode ( Processor_t* processor, FileStat* file );
int  ByteCodeProcessing( Processor_t* processor );
void FillInByteCode    ( Processor_t* processor, char* buffer );


void ProcPush( Processor_t* processor);
void ProcPop ( Processor_t* processor);

void ProcAdd ( Processor_t* processor );
void ProcSub ( Processor_t* processor );
void ProcDiv ( Processor_t* processor );
void ProcMul ( Processor_t* processor );
void ProcPow ( Processor_t* processor );
void ProcSqrt( Processor_t* processor );

void ProcIn ( Processor_t* processor );
void ProcOut( Processor_t* processor );

void ProcPushR( Processor_t* processor );
void ProcPopR ( Processor_t* processor );

void ProcJump( Processor_t* processor );

void ProcCall( Processor_t* processor );
void ProcRet ( Processor_t* processor );

#endif // PROCESSOR_H
