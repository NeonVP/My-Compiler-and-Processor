#include "processor.h"


void ProcPush( Processor_t* processor) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    StackPush( &( processor->stk ), processor->byte_code[ processor->instruction_ptr++ ] );
}

void ProcPop( Processor_t* processor) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    StackPop( &( processor->stk ) );
}

void ProcAdd( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int b = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    StackPush( &( processor->stk ), a + b );
}

void ProcSub( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int b = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    StackPush( &( processor->stk ), a - b );
}

void ProcMul( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int b = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    StackPush( &( processor->stk ), a * b );
}

void ProcDiv( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int b   = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    assert( b != 0 && "Can't divide by zero" );

    StackPush( &( processor->stk ), a / b );
}

void ProcPow( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int indicator = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int base = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    int result = 1;

    for ( int i = 0; i < indicator; i++ ) { // TODO Rename indicator to exp
        result *= base;
    }

    StackPush( &( processor->stk ), result );
}

void ProcSqrt( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    StackPush( &( processor->stk ), ( int ) sqrt( a ) ); // TODO: If we are moving to double, then dont forget to change it
}

void ProcIn( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int number = 0;

    fprintf( stderr, "Input a number: " );
    scanf( "%d", &number );

    StackPush( &( processor->stk ), number );
}

void ProcOut( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int n = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    fprintf( stderr, "Output: %d \n", n );
}

void ProcPushR( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR ); // TODO: Check for out-of-bounds access

    StackPush( &( processor->stk ), processor->regs[ processor->byte_code[ processor->instruction_ptr++ ] ] );
}

void ProcPopR( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    processor->regs[ processor->byte_code[ processor->instruction_ptr++ ] ] = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
}

void ProcJump( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int command = processor->byte_code[ processor->instruction_ptr - 1 ];
    size_t index = ( size_t ) processor->byte_code[ processor->instruction_ptr++ ];

    if ( command == JMP_CMD ) {
        processor->instruction_ptr = index;
        return;
    }

    int b = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    if ( command == JB_CMD && a < b ) {
        processor->instruction_ptr = index;
    }
    else if ( command == JA_CMD && a > b) {
        processor->instruction_ptr = index;
    }
    else if ( command == JBE_CMD && a <= b) {
        processor->instruction_ptr = index;
    }
    else if ( command == JAE_CMD && a >= b) {
        processor->instruction_ptr = index;
    }
    else if ( command == JE_CMD && a == b ) {
        processor->instruction_ptr = index;
    } // TODO: add assert about unknown command in else / else with assert
}

void ProcCall( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    StackPush( &( processor->refund_stk ), ( int ) ( processor->instruction_ptr + 1 ) );        // TODO: refund_stk on size_t

    processor->instruction_ptr = ( size_t ) processor->byte_code[ processor->instruction_ptr ];
}

void ProcRet( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    processor->instruction_ptr = ( size_t ) StackTop( &( processor->refund_stk ) );
    StackPop( &( processor->refund_stk ) );
}

void ProcHlt( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );
    
    processor->is_running = false;
}

void ProcMark( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );
    
    // MARK command does nothing in runtime - it's just a placeholder for labels
    // Just increment instruction pointer to skip the label value
    processor->instruction_ptr++;
}

void ProcLabel( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );
    
    // LABEL command does nothing in runtime - it's just a placeholder for labels
    // Just increment instruction pointer to skip the label value
    processor->instruction_ptr++;
}
