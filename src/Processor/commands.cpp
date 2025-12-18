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

    assert( b != 0 );

    StackPush( &( processor->stk ), a / b );
}

void ProcPow( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int indicator = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    int base = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );

    int result = 1;

    for ( int i = 0; i < indicator; i++ ) {
        result *= base;
    }

    StackPush( &( processor->stk ), result );
}

void ProcSqrt( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    int a = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    StackPush( &( processor->stk ), ( int ) sqrt( a ) );
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
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    StackPush( &( processor->stk ), processor->regs[ processor->byte_code[ processor->instruction_ptr++ ] ] );
}

void ProcPopR( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    processor->regs[ processor->byte_code[ processor->instruction_ptr++ ] ] = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
}

void ProcPushM( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );
    my_assert( processor->RAM, ASSERT_ERR_NULL_PTR );

    // PUSHM [register] или PUSHM address
    // Вытаскиваем значение из RAM по индексу из регистра или прямому адресу
    // байт-код: [PUSHM_CMD, reg_index_or_address]
    int reg_index = processor->byte_code[ processor->instruction_ptr++ ];
    
    int ram_index;
    
    // Если значение < 100, считаем что это регистр (0-3)
    // Если >= 100, считаем что это прямой адрес в RAM
    if ( reg_index < REGS_NUMBER ) {
        // Используем регистр
        ram_index = processor->regs[reg_index];
    } else {
        // Прямой адрес
        ram_index = reg_index - 100;  // смещение для различения
    }
    
    if ( ram_index < 0 || ram_index >= RAM_SIZE ) {
        fprintf( stderr, COLOR_RED "RAM index out of bounds: %d" COLOR_RESET "\n", ram_index );
        assert( 0 && "RAM access violation" );
    }
    
    StackPush( &( processor->stk ), processor->RAM[ram_index] );
}

void ProcPopM( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );
    my_assert( processor->RAM, ASSERT_ERR_NULL_PTR );

    // POPM [register] или POPM address
    // Кладем значение из стека в RAM по индексу из регистра или прямому адресу
    // байт-код: [POPM_CMD, reg_index_or_address]
    int reg_index = processor->byte_code[ processor->instruction_ptr++ ];
    
    int ram_index;
    
    // Если значение < 100, считаем что это регистр (0-3)
    // Если >= 100, считаем что это прямой адрес в RAM
    if ( reg_index < REGS_NUMBER ) {
        // Используем регистр
        ram_index = processor->regs[reg_index];
    } else {
        // Прямой адрес (вычитаем 100 для получения реального адреса)
        ram_index = reg_index - 100;
    }
    
    if ( ram_index < 0 || ram_index >= RAM_SIZE ) {
        fprintf( stderr, COLOR_RED "RAM index out of bounds: %d" COLOR_RESET "\n", ram_index );
        assert( 0 && "RAM access violation" );
    }
    
    int value = StackTop( &( processor->stk ) );
    StackPop( &( processor->stk ) );
    
    processor->RAM[ram_index] = value;
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
    }
}

void ProcCall( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    StackPush( &( processor->refund_stk ), ( int ) ( processor->instruction_ptr + 1 ) );

    processor->instruction_ptr = ( size_t ) processor->byte_code[ processor->instruction_ptr ];
}

void ProcRet( Processor_t* processor ) {
    my_assert( processor, ASSERT_ERR_NULL_PTR );

    processor->instruction_ptr = ( size_t ) StackTop( &( processor->refund_stk ) );
    StackPop( &( processor->refund_stk ) );
}