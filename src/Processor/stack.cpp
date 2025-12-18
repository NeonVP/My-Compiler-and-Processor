#include "stack.h"

void StackCtor( Stack_t* stk, size_t capacity ) {
    ON_DEBUG(
        if ( capacity > large_capacity ) {
            stk->varInfo.err_code |= ERR_LARGE_CAPACITY;
        }
    )

    stk->data = ( int* ) calloc ( capacity ON_CANARY( + 2 ), sizeof( StackData_t ) );
    assert( stk->data != NULL );

    stk->capacity = capacity;
    stk->size     = 0;

    #ifdef _CANARY
        *( stk->data ) = canary;
        *( stk->data + stk->capacity + 1 ) = canary;

        stk->data++;
    #endif

    StackToPoison( stk );
}

void StackDtor( Stack_t* stk ) {
    ON_DEBUG( StackVerify( stk ); )

    ON_CANARY( stk->data--; )
    free( stk->data );

    stk->data     = NULL;
    stk->capacity = 0;
    stk->size     = 0;
}

void StackPush( Stack_t* stk, int element ) {
    // PRINT( "%s %d \n", __func__, element )
    DEBUG_IN_FUNC(
        if ( stk->size == stk->capacity ) {
            StackRealloc( stk, stk->capacity * 2 );
        }

        *( stk->data + stk->size ) = element;
        stk->size++;
    )
}

void StackPop( Stack_t* stk ) {
    // PRINT( "%s \n", __func__ )
    DEBUG_IN_FUNC(
        stk->size--;
        *( stk->data + stk->size ) = poison;

        if ( stk->size * 4 <= stk->capacity && stk->size != 0 ) {
            StackRealloc( stk, stk->capacity / 2 + 1 );
        }
    )
}

StackData_t StackTop( Stack_t* stk ) {
    // PRINT( "%s \n", __func__ )

    #ifdef _DEBUG
    if ( StackVerify( stk ) != ERR_NONE ) {
        return 0;
    }
    #endif

    return *( stk->data + stk->size - 1 );
}

void StackRealloc( Stack_t* stk, size_t capacity ) {
    // DEBUG_IN_FUNC(
        stk->capacity = capacity;
        ON_CANARY( stk->data--; )

        StackData_t* new_ptr = ( int* ) realloc ( stk->data, ( capacity ON_CANARY( + 2 ) ) * sizeof( StackData_t ) );
        assert( new_ptr && "Memory reallocation error \n" );
        stk->data = new_ptr;

        #ifdef _CANARY
            *( stk->data ) = canary;
            *( stk->data + stk->capacity + 1 ) = canary;

            stk->data++;
        #endif

        StackToPoison( stk );
    // )
}

void StackToPoison( Stack_t* stk ) {
    DEBUG_IN_FUNC(
        for ( size_t i = stk->size; i < stk->capacity; i++ ) {
            *( stk->data + i ) = poison;
        }
    )
}

#ifdef _DEBUG
long StackVerify( Stack_t* stk ) {
    if ( stk == NULL ) {
        stk->varInfo.err_code |= ERR_BAD_PTR_STRUCT;
    }

    if ( stk->data == NULL ) {
        stk->varInfo.err_code |= ERR_BAD_PTR_DATA;
    }

    #ifdef _CANARY
    if ( stk->data != NULL ) {
        if ( *( stk->data - 1 ) != canary || *( stk->data + stk->capacity ) != canary ) {
            stk->varInfo.err_code |= ERR_CORRUPTED_CANARY_DATA;
        }

        if ( stk->canary1 != canary || stk->canary2 != canary ) {
            stk->varInfo.err_code |= ERR_CORRUPTED_CANARY_STRUCT;
        }
    }
    #endif

    if ( stk->capacity < stk->size ) {
        stk->varInfo.err_code |= ERR_SIZE_OVER_CAPACITY;
    }
    else if ( stk->data != NULL ) {
        for ( size_t i = 0; i < stk->size; i++ ) {
            if ( *( stk->data + i ) == poison ) {
                stk->varInfo.err_code |= ERR_POISON_IN_FILLED_CELLS;
                break;
            }
        }
    }

    if ( stk->capacity > large_capacity ) {
        stk->varInfo.err_code |= ERR_LARGE_CAPACITY;
    }

    if ( stk->varInfo.err_code != ERR_NONE ) {
        StackDump( stk );
    }

    return stk->varInfo.err_code;
}

void ErrorProcessing( long err_code ) {
    if ( err_code & ERR_BAD_PTR_STRUCT ) {
        printerr( "\tERROR: INVALID pointer to the structure with parameters  \n" );
        return;
    }

    if ( err_code & ERR_BAD_PTR_DATA ) {
        printerr( "\tERROR: INVALID pointer to the stack \n" );
    }

    if ( err_code & ERR_CORRUPTED_CANARY_DATA ) {
        printerr( "\tERROR: Corrupted canaries in data \n" );
    }

    if ( err_code & ERR_CORRUPTED_CANARY_STRUCT ) {
        printerr( "\tERROR: Corrupted canaries in struct of stack \n" );
    }

    if ( err_code & ERR_SIZE_OVER_CAPACITY ) {
        printerr( "\tERROR: Size over capacity \n" );
    }

    if ( err_code & ERR_POISON_IN_FILLED_CELLS ) {
        printerr( "\tERROR: Poison in filled cells \n" );
    }

    if ( err_code & ERR_LARGE_CAPACITY ) {
        printerr( "\tWARNING: Too big capacity \n" );
    }
}


void StackDump( const Stack_t* stk ) {
    if ( stk == NULL ) {
        printerr( "Error in DUMP: Null Pointer on stack STRUCT\n" );
    }
    else {
        fprintf( stderr, COLOR_YELLOW "stack<int>[%p]" ON_DEBUG( " --- name: \"%s\" --- FILE: %s --- LINE: %lu --- FUNC: %s" ) "\n" COLOR_RESET,
                 stk ON_DEBUG( , stk->varInfo.name, stk->varInfo.file, stk->varInfo.line, stk->varInfo.func ) );

        ON_DEBUG(
            if (  stk->varInfo.err_code != ERR_NONE ) {
                fprintf( stderr, COLOR_RED "  ERROR CODE %ld:\n" COLOR_RED, stk->varInfo.err_code );
                ErrorProcessing( stk->varInfo.err_code );
            }
        )

        fprintf( stderr, COLOR_CYAN
                         "  {              \n"
                         "  capacity = %lu\n"
                         "  size     = %lu\n"
                         "  data     = %p \n"
                         COLOR_RESET,
                         stk->capacity,
                         stk->size,
                         stk->data );

        if ( stk->data == NULL || stk->size > stk->capacity ) {
            PASS;
        }
        else {
            fprintf( stderr, "\t{\n" );

            for ( size_t i = 0; i < stk->capacity; i++ ) {
                if ( *( stk->data + i ) != poison ) {
                    fprintf( stderr, "\t*[%lu] = %d\n", i, *( stk->data + i ) );
                }
                else {
                    fprintf( stderr, "\t [%lu] = %d ( poison ) \n", i, *( stk->data + i ) );
                }
            }

            fprintf( stderr, "\t}\n" );
        }

        fprintf( stderr, COLOR_CYAN "  }\n" COLOR_RESET );
    }
}
#endif
