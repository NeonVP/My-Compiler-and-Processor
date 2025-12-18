#include "assembler.h"

#define StrCompare( str1, str2 ) strncmp( str1, str2, sizeof( str2 ) - 1 )
#define FREE_BUF_AND_STRINGS free( buffer ); free( strings );

const int SUCCESS_RESULT = 1;
const int FAIL_RESULT = 0;
const int NOT_REGISTER = -1;

ON_DEBUG( void PrintLabels( Assembler_t* assembler ); )

// =================================
// Новые функции для меток
// =================================

int FindLabelAddress( const Assembler_t* assembler, const char* label_name ) {
    my_assert( assembler,   ASSERT_ERR_NULL_PTR );
    my_assert( label_name,  ASSERT_ERR_NULL_PTR );
    
    for ( size_t i = 0; i < assembler->labels_count; i++ ) {
        if ( strcmp( assembler->labels[i].name, label_name ) == 0 ) {
            return assembler->labels[i].address;
        }
    }
    
    return -1;  // Метка не найдена
}

int AddLabel( Assembler_t* assembler, const char* label_name, int address ) {
    my_assert( assembler,   ASSERT_ERR_NULL_PTR );
    my_assert( label_name,  ASSERT_ERR_NULL_PTR );
    
    if ( assembler->labels_count >= LABELS_NUMBER ) {
        fprintf( stderr, COLOR_BRIGHT_RED "Maximum number of labels (%lu) exceeded\n" COLOR_RESET, LABELS_NUMBER );
        return FAIL_RESULT;
    }
    
    // Проверяем, что метка ещё не существует
    if ( FindLabelAddress( assembler, label_name ) != -1 ) {
        fprintf( stderr, COLOR_BRIGHT_RED "Label '%s' already defined\n" COLOR_RESET, label_name );
        return FAIL_RESULT;
    }
    
    strncpy( assembler->labels[assembler->labels_count].name, label_name, MAX_LABEL_LEN - 1 );
    assembler->labels[assembler->labels_count].name[MAX_LABEL_LEN - 1] = '\0';
    assembler->labels[assembler->labels_count].address = address;
    assembler->labels_count++;
    
    return SUCCESS_RESULT;
}

void AssemblerCtor( Assembler_t* assembler, int argc, char** argv ) {
    my_assert( assembler,        ASSERT_ERR_NULL_PTR        );
    my_assert( argv,             ASSERT_ERR_NULL_PTR        );

    ArgvProcessing( argc, argv, &( assembler->asm_file ), &( assembler->exe_file ) );

    // Очищаем массив меток
    for ( size_t i = 0; i < LABELS_NUMBER; i++ ) {
        assembler->labels[i].name[0] = '\0';
        assembler->labels[i].address = -1;
    }
    assembler->labels_count = 0;
}

void AssemblerDtor( Assembler_t* assembler ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );

    free( assembler->byte_code );
    assembler->byte_code = NULL;

    free( assembler->asm_file.address );
    assembler->asm_file.address = NULL;

    free( assembler->exe_file.address );
    assembler->exe_file.address = NULL;
}

int AsmCodeToByteCode( Assembler_t* assembler ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );

    PRINT( GRID COLOR_BRIGHT_YELLOW "In %s \n\n", __func__ )

    char* buffer = ReadToBuffer( &( assembler->asm_file ) );

    StrPar* strings = ( StrPar* ) calloc ( assembler->asm_file.nLines, sizeof( *strings ) );
    assert( strings && "Error in memory allocation for \"strings\" \n" );

    assembler->asm_file.nLines = SplitIntoLines( strings, buffer, assembler->asm_file.nLines );

    // Каждая строка может содержать максимум 2 инструкции
    assembler->byte_code = ( int* ) calloc ( assembler->asm_file.nLines * 2, sizeof( int ) );
    assert( assembler->byte_code && "Error in memory allocation for \"byte-code\" \n" );

    int translate_result = 1;

    PRINT( COLOR_BRIGHT_YELLOW "\n  ---First Run--- \n" );
    translate_result = TranslateAsmToByteCode( assembler, strings );
    ON_DEBUG( PrintLabels( assembler ); )
    if ( translate_result == 0 ) { FREE_BUF_AND_STRINGS; return 0; }

    PRINT( COLOR_BRIGHT_YELLOW "\n  ---Second Run---    \n");
    translate_result = TranslateAsmToByteCode( assembler, strings );
    ON_DEBUG( PrintLabels( assembler ); )
    FREE_BUF_AND_STRINGS;

    PRINT( "\n" GRID COLOR_BRIGHT_YELLOW "Out %s \n", __func__ )

    return translate_result;
}

int TranslateAsmToByteCode( Assembler_t* assembler, StrPar* strings ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );
    my_assert( strings,   ASSERT_ERR_NULL_PTR );

    assembler->instruction_cnt = 0;

    char instruction[ MAX_INSTRUCT_LEN ] = "";
    char label_name[ MAX_LABEL_LEN ] = "";
    Argument argument = {};

    int command          = 0;
    int number_of_params = 0;
    int number_of_characters_read = 0;
    const char* str_pointer = 0;
    int HLT_flag = 0;

    for ( size_t i = 0; i < assembler->asm_file.nLines; i++ ) {
        str_pointer = strings[i].ptr;
        
        // Пропускаем пустые строки и комментарии
        if ( str_pointer[0] == ';' || str_pointer[0] == '\0' ) continue;
        
        number_of_params = sscanf( str_pointer, "%s%n", instruction, &number_of_characters_read );
        if ( number_of_params == 0 ) continue;
        
        // Пропускаем, если первый токен - комментарий
        if ( instruction[0] == ';' ) continue;

        str_pointer += number_of_characters_read;
        command = AsmCodeProcessing( instruction );
        
        // Обрезаем комментарии
        char* comment_ptr = strchr( (char*)str_pointer, ';' );
        if ( comment_ptr ) {
            *comment_ptr = '\0';
        }
        
        ArgumentProcessing( &argument, str_pointer );

        switch ( command ) {
            case MARK_CMD:
                // Парсим название метки (:label_name)
                strncpy( label_name, instruction + 1, MAX_LABEL_LEN - 1 );
                label_name[MAX_LABEL_LEN - 1] = '\0';
                
                // Добавляем метку
                if ( AddLabel( assembler, label_name, (int)assembler->instruction_cnt ) == SUCCESS_RESULT ) {
                    PRINT( COLOR_BRIGHT_GREEN "%-10s\n", strings[i].ptr );
                } else {
                    return FAIL_RESULT;
                }
                break;

            case PUSH_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                if ( argument.type == NUMBER ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, command, argument.value );
                } else {
                    fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for PUSH in file: %s:%lu (expected NUMBER)\n", assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                break;

            case PUSHR_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                if ( argument.type == REGISTER ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, command, argument.value );
                } else {
                    fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for PUSHR in file: %s:%lu (expected REGISTER)\n", assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                break;

            case POP_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                if ( argument.type != VOID ) {
                    fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for POP in file: %s:%lu (expected no arguments)\n", assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, command );
                break;

            case POPR_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                if ( argument.type == REGISTER ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, command, argument.value );
                } else {
                    fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for POPR in file: %s:%lu (expected REGISTER)\n", assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                break;

            case PUSHM_CMD:
            case POPM_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                if ( argument.type == REGISTER ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d (via register)\n", strings[i].ptr, command, argument.value );
                } else if ( argument.type == NUMBER ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value + 100;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d (direct address)\n", strings[i].ptr, command, argument.value + 100 );
                } else {
                    fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for %s in file: %s:%lu (expected [REGISTER] or NUMBER)\n", 
                             (command == PUSHM_CMD) ? "PUSHM" : "POPM", assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                break;

            case ADD_CMD:
            case SUB_CMD:
            case MUL_CMD:
            case DIV_CMD:
            case POW_CMD:
            case SQRT_CMD:
            case IN_CMD:
            case OUT_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                if ( argument.type != VOID ) {
                    fprintf( stderr, COLOR_BRIGHT_RED "Incorrect command in file: %s:%lu \n", assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }

                PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, command );
                break;

            case JMP_CMD:
            case JE_CMD:
            case JB_CMD:
            case JA_CMD:
            case JBE_CMD:
            case JAE_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                // Парсим название метки
                char target_label[MAX_LABEL_LEN] = "";
                sscanf( str_pointer, ":%s", target_label );
                
                int target_address = FindLabelAddress( assembler, target_label );
                if ( target_address != -1 ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = target_address;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 2 ],
                                                                                      assembler->byte_code[ assembler->instruction_cnt - 1 ] );
                } else {
                    fprintf( stderr, COLOR_BRIGHT_RED "Label '%s' not found in file: %s:%lu\n", target_label, assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                break;

            case CALL_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                // Парсим название метки для CALL
                char func_label[MAX_LABEL_LEN] = "";
                sscanf( str_pointer, ":%s", func_label );
                
                int func_address = FindLabelAddress( assembler, func_label );
                if ( func_address != -1 ) {
                    assembler->byte_code[ assembler->instruction_cnt++ ] = func_address;
                    PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 2],
                                                                                      assembler->byte_code[ assembler->instruction_cnt - 1 ] );
                } else {
                    fprintf( stderr, COLOR_BRIGHT_RED "Function '%s' not found in file: %s:%lu\n", func_label, assembler->asm_file.address, i + 1 );
                    return FAIL_RESULT;
                }
                break;

            case RET_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;

                PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 1 ] );
                break;

            case HLT_CMD:
                assembler->byte_code[ assembler->instruction_cnt++ ] = command;
                HLT_flag++;

                PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 1 ] );
                break;

            default:
                fprintf( stderr, COLOR_BRIGHT_RED "Incorrect command \"%s\" in file: %s:%lu \n", strings[i].ptr, assembler->asm_file.address, i + 1 );

                return FAIL_RESULT;
        }
    }

    if ( HLT_flag ) return SUCCESS_RESULT;

    fprintf( stderr, COLOR_BRIGHT_RED "There is no HLT command \n" COLOR_RESET );
    return FAIL_RESULT;
}

int AsmCodeProcessing( char* instruction ) {
    my_assert( instruction, ASSERT_ERR_NULL_PTR );

    if ( instruction[0] == ':' )                    { return MARK_CMD; }
    
    if ( StrCompare( instruction, "PUSHM" ) == 0 ) { return PUSHM_CMD; }
    if ( StrCompare( instruction, "PUSHR" ) == 0 ) { return PUSHR_CMD; }
    if ( StrCompare( instruction, "PUSH" ) == 0 )  { return PUSH_CMD; }
    if ( StrCompare( instruction, "POPM" ) == 0 )  { return POPM_CMD; }
    if ( StrCompare( instruction, "POPR" ) == 0 )  { return POPR_CMD;  }
    if ( StrCompare( instruction, "POP"  ) == 0 )  { return POP_CMD;  }
    if ( StrCompare( instruction, "ADD"  ) == 0 )  { return ADD_CMD;  }
    if ( StrCompare( instruction, "SUB"  ) == 0 )  { return SUB_CMD;  }
    if ( StrCompare( instruction, "MUL"  ) == 0 )  { return MUL_CMD;  }
    if ( StrCompare( instruction, "DIV"  ) == 0 )  { return DIV_CMD;  }
    if ( StrCompare( instruction, "POW"  ) == 0 )  { return POW_CMD;  }
    if ( StrCompare( instruction, "SQRT" ) == 0 )  { return SQRT_CMD; }
    if ( StrCompare( instruction, "IN"   ) == 0 )  { return IN_CMD;   }
    if ( StrCompare( instruction, "OUT"  ) == 0 )  { return OUT_CMD;  }
    if ( StrCompare( instruction, "CALL" ) == 0 )  { return CALL_CMD; }
    if ( StrCompare( instruction, "RET"  ) == 0 )  { return RET_CMD;  }

    if ( StrCompare( instruction, "JMP"  ) == 0 )  { return JMP_CMD;  }
    if ( StrCompare( instruction, "JE"   ) == 0 )  { return JE_CMD;   }
    if ( StrCompare( instruction, "JB"   ) == 0 )  { return JB_CMD;   }
    if ( StrCompare( instruction, "JA"   ) == 0 )  { return JA_CMD;   }
    if ( StrCompare( instruction, "JBE"  ) == 0 )  { return JBE_CMD;  }
    if ( StrCompare( instruction, "JAE"  ) == 0 )  { return JAE_CMD;  }

    if ( StrCompare( instruction, "HLT"  ) == 0 )  { return HLT_CMD;  }

    return FAIL_RESULT;
}

void OutputInFile( Assembler_t* assembler ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );

    FILE* file = fopen( assembler->exe_file.address, "w" );
    my_assert( file, ASSERT_ERR_FAIL_OPEN );

    fprintf( file, "%lu", assembler->instruction_cnt );

    for ( size_t i = 0; i < assembler->instruction_cnt; i++ ) {
        fprintf( file, " %d", assembler->byte_code[i] );
    }

    int result_of_fclose = fclose( file );
    my_assert( result_of_fclose == 0, ASSERT_ERR_FAIL_CLOSE )
}

int ArgumentProcessing( Argument* argument, const char* string ) {
    my_assert( string,   ASSERT_ERR_NULL_PTR );
    my_assert( argument, ASSERT_ERR_NULL_PTR );

    int number_of_elements_read = 0;
    argument->type = VOID;
    argument->value = -1;

    while ( *string && isspace( *string ) ) string++;
    
    if ( !*string || *string == ';' ) {
        argument->type = VOID;
        return -1;
    }

    number_of_elements_read = sscanf( string, "%d", &( argument->value ) );
    if ( number_of_elements_read == 1 ) {
        argument->type = NUMBER;
        return argument->value;
    }

    char instruction[MAX_LABEL_LEN] = "";
    number_of_elements_read = sscanf( string, "%s", instruction );
    if ( number_of_elements_read == 1 ) {
        // Проверяем метки с двоеточием (:label_name)
        if ( instruction[0] == ':' ) {
            // Метка обнаружена, но адрес будет заполнен после
            // Во втором проходе
            argument->type = LABEL;
            argument->value = 0;  // Временно, значение будет заменено
            return 0;
        }

        if ( instruction[0] == '[' && instruction[strlen(instruction)-1] == ']' ) {
            char reg[MAX_LABEL_LEN] = "";
            strncpy( reg, instruction + 1, strlen(instruction) - 2 );
            reg[strlen(instruction) - 2] = '\0';
            
            if ( strlen( reg ) == 3 && reg[0] == 'R' && reg[2] == 'X' ) {
                argument->value = reg[1] - 'A' + 1;
                if ( argument->value >= 1 && argument->value <= 26 ) {
                    argument->type = REGISTER;
                    return argument->value;
                }
            }
            
            if ( strlen( reg ) == 2 && reg[1] == 'X' && isalpha( reg[0] ) ) {
                argument->value = reg[0] - 'A' + 1;
                if ( argument->value >= 1 && argument->value <= 26 ) {
                    argument->type = REGISTER;
                    return argument->value;
                }
            }
        }

        if ( strlen( instruction ) == 3 && instruction[0] == 'R' && instruction[2] == 'X' ) {
            argument->value = instruction[1] - 'A' + 1;
            if ( argument->value >= 1 && argument->value <= 26 ) {
                argument->type = REGISTER;
                return argument->value;
            }
        }
        
        if ( strlen( instruction ) == 2 && instruction[1] == 'X' && isalpha( instruction[0] ) ) {
            argument->value = instruction[0] - 'A' + 1;
            if ( argument->value >= 1 && argument->value <= 26 ) {
                argument->type = REGISTER;
                return argument->value;
            }
        }
    }
    else {
        argument->type = VOID;
        return -1;
    }

    PRINT( "%s\n", instruction );
    argument->type = UNKNOWN;
    argument->value = -1;
    return -1;
}

#ifdef _DEBUG
void PrintLabels( Assembler_t* assembler ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );

    PRINT( COLOR_BLUE "Labels: \n" );
    for ( size_t i = 0; i < assembler->labels_count; i++ ) {
        PRINT( COLOR_BRIGHT_CYAN "%-32s - %d \n", assembler->labels[i].name, assembler->labels[i].address );
    }
}
#endif
