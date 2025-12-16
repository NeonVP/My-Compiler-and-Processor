#include "processor.h"

void ProcCtor( Processor_t* processor, size_t stack_size, size_t refund_stack_size ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR )

    StackCtor( &( processor->stk ), stack_size );
    StackCtor( &( processor->refund_stk ), refund_stack_size );
}

void ProcDtor( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR )

    StackDtor( &( processor->stk        ) );
    StackDtor( &( processor->refund_stk ) );
    free( processor->byte_code );
    processor->byte_code = NULL;
}

// ProcessorStatus_t ProcVerify( Processor_t* processor ) {                // TODO: add Verify!!!

// }

#ifdef _DEBUG
ProcessorStatus_t ProcDump( const Processor_t* processor, const int error ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    printf( COLOR_BRIGHT_YELLOW "+=+=+=+=+=+=+=+=+=+ PROCESSOR +=+=+=+=+=+=+=+=+=+ \n" );
    printf( COLOR_CYAN          "Processor address: " COLOR_RESET "%p \n", processor );
    printf( COLOR_CYAN          "Byte code pointer: " COLOR_RESET "%p \n", (void*)processor->byte_code );

    PrintByteCodeInline( processor );
    PrintRegisters( processor );

    PRINT( COLOR_GREEN "\nMain Stack:\n" );
    ON_DEBUG( StackDump( &( processor->stk ) ); )

    PRINT( COLOR_GREEN "\nRefund Stack:\n" );
    StackDump( &( processor->refund_stk ) );

    PRINT( COLOR_BRIGHT_YELLOW "+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ \n" );

    return SUCCESS;
}

void PrintByteCodeInline(const Processor_t* processor) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    PRINT( COLOR_BRIGHT_YELLOW "\nBytecode:\n" );

    for ( size_t i = 0; i < processor->instruction_count; i++ ) {   // TODO: instruction_idx (rename)
        if ( i == processor->instruction_ptr ) {
            PRINT( COLOR_BRIGHT_RED ">%d< ", processor->byte_code[i] );
        } else {
            PRINT( " %d  ", processor->byte_code[i] );
        }
    }
    PRINT( "\n" );

    PRINT( COLOR_BRIGHT_CYAN "instruction_ptr = %lu; instruction_count = %lu\n",
           processor->instruction_ptr, processor->instruction_count );
}

void PrintRegisters( const Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    PRINT( COLOR_BRIGHT_MAGENTA "\nRegisters:\n" );

    for ( size_t i = 0; i < REGS_NUMBER; i++ ) {
        PRINT( " R%cX = %5d  ", ( i == 0 ) ? 'O' : ( int ) ( 'A' - 1 + i ), processor->regs[i] );

        if ( ( i + 1 ) % 5 == 0 ) PRINT( "\n" );        // TODO: index_print_offset (5)
    }

    if ( REGS_NUMBER % 5 != 0 ) PRINT( "\n" );
}
#endif

void ExeFileToByteCode( Processor_t* processor, FileStat* file ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );
    my_assert( file,      ASSERT_ERR_NULL_PTR );

    PRINT( COLOR_BRIGHT_YELLOW "In %s \n", __func__ )

    file->size = DetermineFileSize( file->address );

    char* buffer = ReadToBuffer( file );
    char* old_buffer_ptr = buffer;

    int    number_of_characters_read = 0;
    sscanf( buffer, "%lu%n", &( processor->instruction_count ), &number_of_characters_read );
    buffer += number_of_characters_read;

    processor->byte_code = ( int* ) calloc ( processor->instruction_count, sizeof( *processor->byte_code ) );
    assert( processor->byte_code && "Memory allocation error \n" );

    FillInByteCode( processor, buffer );

    fprintf( stderr, "\n" );

    free( old_buffer_ptr );

    PRINT( COLOR_BRIGHT_YELLOW "Out %s \n", __func__ )
}

void FillInByteCode( Processor_t* processor, char* buffer ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int instruction = 0;        // TODO: current instruction
    int number_of_characters_read = 0;

    for ( size_t i = 0; i < processor->instruction_count; i++ ) {
        if ( sscanf( buffer, "%d %n", &instruction, &number_of_characters_read ) == 1 ) {
            processor->byte_code[ i ] = instruction;
            buffer += number_of_characters_read;
            PRINT( " %d", instruction )
        }
    }
}

int ByteCodeProcessing( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR )

    PRINT( COLOR_BRIGHT_YELLOW "In %s \n", __func__ );

    ON_DEBUG( ProcDump( processor, 0 ) );

    int command = 0;
    while ( processor->instruction_ptr < processor->instruction_count ) {
        command = processor->byte_code[ processor->instruction_ptr++ ];

        switch ( command ) {
            case PUSH_CMD:  ProcPush ( processor ); break;
            case POP_CMD:   ProcPop  ( processor ); break;
            case ADD_CMD:   ProcAdd  ( processor ); break;
            case SUB_CMD:   ProcSub  ( processor ); break;
            case MUL_CMD:   ProcMul  ( processor ); break;
            case DIV_CMD:   ProcDiv  ( processor ); break;
            case POW_CMD:   ProcPow  ( processor ); break;
            case SQRT_CMD:  ProcSqrt ( processor ); break;
            case IN_CMD:    ProcIn   ( processor ); break;
            case OUT_CMD:   ProcOut  ( processor ); break;
            case PUSHR_CMD: ProcPushR( processor ); break;
            case POPR_CMD:  ProcPopR ( processor ); break;

            case CALL_CMD:  ProcCall ( processor ); break;
            case RET_CMD:   ProcRet  ( processor ); break;

            case JMP_CMD:
            case JE_CMD:
            case JB_CMD:
            case JA_CMD:
            case JBE_CMD:
            case JAE_CMD:   ProcJump( processor ); break;

            case HLT_CMD:   return 0;

            default:
                fprintf( stderr, COLOR_RED "Incorrect command %d \n" COLOR_RESET, *( processor->byte_code ) );
                return 1;
        }

        ON_DEBUG( ProcDump( processor, 0 ); )
        PRINT( COLOR_BLUE "Press enter for next step... \n" );
        ON_DEBUG( getchar(); )
    }

    PRINT( COLOR_BRIGHT_YELLOW "Out %s \n", __func__ )

    return 0;
}
