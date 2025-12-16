#include "Compiler.h"
#include "AssertUtils.h"
#include "supported_commands.h"

#define StrCompare( str1, str2 ) strncmp( str1, str2, sizeof( str2 ) - 1 )
#define FREE_BUF_AND_STRINGS free( buffer ); free( strings );

const int SUCCESS_RESULT = 1;
const int FAIL_RESULT = 0;

void AssemblerCtor( Assembler_t* assembler, int argc, char** argv ) {
    my_assert( assembler,        ASSERT_ERR_NULL_PTR        );
    my_assert( isfinite( argc ), ASSERT_ERR_INFINITE_NUMBER );
    my_assert( argv,             ASSERT_ERR_NULL_PTR        );

    ArgvProcessing( argc, argv, &( assembler->asm_file ), "./asm-code.txt", 
                                &( assembler->exe_file ), "./byte-code.txt" );

    for ( size_t i = 0; i < LABELS_NUMBER; i++ ) assembler->labels[ i ] = -1;
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

    // Every row have maximum 2 instructions, so max number of instructions is number of lines twice
    assembler->byte_code = ( int* ) calloc ( assembler->asm_file.nLines * 2, sizeof( int ) );
    assert( assembler->byte_code && "Error in memory allocation for \"byte-code\" \n" );

    int translate_result = 1;

    PRINT( COLOR_BRIGHT_YELLOW "\n  ---First Run--- \n" );
    translate_result = TranslateAsmToByteCode( assembler, strings );
    ON_DEBUG( PrintLabels( assembler->labels ); )
    if ( translate_result == 0 ) { FREE_BUF_AND_STRINGS; return 0; }

    PRINT( COLOR_BRIGHT_YELLOW "\n  ---Second Run---    \n");
    translate_result = TranslateAsmToByteCode( assembler, strings );
    ON_DEBUG( PrintLabels( assembler->labels ); )
    FREE_BUF_AND_STRINGS;

    PRINT( "\n" GRID COLOR_BRIGHT_YELLOW "Out %s \n", __func__ )

    return translate_result;
}

int SecondPass( Assembler_t* assembler, StrPar* strings ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );
    my_assert( strings,   ASSERT_ERR_NULL_PTR );

    assembler->instruction_cnt = 0;

    char* instruction = NULL;
    ArgumentStat argument = {};

    int number_of_params          = 0;
    int number_of_characters_read = 0;
    const char* str_pointer = 0;
    int flag_HLT = 0;
    
    for ( size_t string_idx = 0; string_idx < assembler->asm_file.nLines; string_idx++ ) {
        str_pointer = strings[ string_idx ].ptr;
        number_of_params = sscanf( str_pointer, "%ms%n", &instruction, &number_of_characters_read );
        if ( number_of_params == 0 ) continue;

        str_pointer += number_of_characters_read;

        for ( size_t idx = 0; idx < number_of_commands; idx++ ) {
            if ( StrCompare( instruction, commands[ idx ].command_name ) == 0 && argument.type == commands[ idx ].type_of_param ) {
                assembler->byte_code[ assembler->instruction_cnt ] = commands[ idx ].command_number;
                if ( assembler->byte_code[ assembler->instruction_cnt++ ] == HLT_CMD ) flag_HLT++;
                
                if ( commands[ idx ].command_number == LABEL_CMD || commands[ idx ].command_number == MARK_CMD ) {
                    // Handle label marking
                    continue; // Skip adding argument for label commands
                }

                assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
                break; // Found and processed the command, move to next instruction
            }
        }

        free(instruction); // Free the allocated instruction string
    }
    
    return flag_HLT > 0 ? SUCCESS_RESULT : FAIL_RESULT;
}

// int TranslateAsmToByteCode( Assembler_t* assembler, StrPar* strings ) { // FIXME SHIIIIT. VERY BAD. !!!!!!!!!!!!!!!!!!
//     my_assert( assembler, ASSERT_ERR_NULL_PTR );
//     my_assert( strings,   ASSERT_ERR_NULL_PTR );

//     assembler->instruction_cnt = 0;

//     char instruction[ MAX_INSTRUCT_LEN ] = "";
//     Argument argument = {};

//     int command          = 0;
//     int number_of_params = 0;
//     int number_of_characters_read = 0;
//     const char* str_pointer = 0;
//     int HLT_flag = 0;       // TODO: rename

//     for ( size_t i = 0; i < assembler->asm_file.nLines; i++ ) {
//         str_pointer = strings[i].ptr;
//         number_of_params = sscanf( str_pointer, "%s%n", instruction, &number_of_characters_read );
//         if ( number_of_params == 0 ) continue;

//         str_pointer += number_of_characters_read;
//         command = AsmCodeProcessing( instruction );
//         ArgumentProcessing( &argument, str_pointer );

//         switch ( command ) {
//             case MARK_CMD:
//                 ArgumentProcessing( &argument, instruction );

//                 if ( argument.type == MARK ) {
//                     assembler->labels[ argument.value ] = ( int ) assembler->instruction_cnt;

//                     PRINT( COLOR_BRIGHT_GREEN "%-10s\n", strings[i].ptr );
//                     break;
//                 }
//                 else {
//                     fprintf( stderr, "Incorrect mark in file: %s:%lu\n", assembler->asm_file.address, i + 1 );
//                     return FAIL_RESULT;
//                 }

//             case PUSH_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;

//                 switch ( argument.type ) {
//                     case NUMBER:
//                         assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
//                         break;

//                     case REGISTER:
//                         assembler->byte_code[ assembler->instruction_cnt - 1 ] += 32;
//                         assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
//                         break;

//                     case VOID:
//                     case UNKNOWN:
//                     case MARK:
//                     default:
//                         fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for PUSH in file: %s:%lu \n", assembler->asm_file.address, i + 1 );
//                         return FAIL_RESULT;
//                 }

//                 PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 2 ],
//                                                                                   assembler->byte_code[ assembler->instruction_cnt - 1 ] );

//                 break;

//             case POP_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;

//                 switch ( argument.type ) {
//                     case REGISTER:
//                         assembler->byte_code[ assembler->instruction_cnt - 1 ] += 32;
//                         assembler->byte_code[ assembler->instruction_cnt++ ] = argument.value;
//                         break;

//                     case VOID:
//                         break;

//                     case UNKNOWN:
//                     case NUMBER:
//                     case MARK:
//                     default:
//                         fprintf( stderr, COLOR_BRIGHT_RED "Incorrect argument for POP in file: %s:%lu \n", assembler->asm_file.address, i + 1 );
//                         return FAIL_RESULT;
//                 }

//                 if ( argument.type == REGISTER ) {
//                     PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 2 ],
//                                                                                       assembler->byte_code[ assembler->instruction_cnt - 1 ] );
//                 }
//                 else {
//                     PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 1 ] );
//                 }

//                 break;

//             case ADD_CMD:
//             case SUB_CMD:
//             case MUL_CMD:
//             case DIV_CMD:
//             case POW_CMD:
//             case SQRT_CMD:
//             case IN_CMD:
//             case OUT_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;

//                 if ( argument.type != VOID ) {
//                     fprintf( stderr, COLOR_BRIGHT_RED "Incorrect command in file: %s:%lu \n", assembler->asm_file.address, i + 1 );
//                     return FAIL_RESULT;
//                 }

//                 PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, command );
//                 break;

//             case JMP_CMD:
//             case JE_CMD:
//             case JB_CMD:
//             case JA_CMD:
//             case JBE_CMD:
//             case JAE_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;

//                 switch ( argument.type ) {
//                     case MARK:
//                         assembler->byte_code[ assembler->instruction_cnt++ ] = assembler->labels[ argument.value ];
//                         break;

//                     case VOID:
//                     case UNKNOWN:
//                     case NUMBER:
//                     case REGISTER:
//                     default:
//                         fprintf( stderr, COLOR_BRIGHT_RED "Incorrect mark for JMP in file: %s:%lu\nType - %d\n", assembler->asm_file.address, i + 1, argument.type );
//                         return FAIL_RESULT;
//                 }

//                 PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 2 ],
//                                                                                   assembler->byte_code[ assembler->instruction_cnt - 1 ] );
//                 break;

//             case CALL_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;

//                 switch ( argument.type ) {
//                     case MARK:
//                         assembler->byte_code[ assembler->instruction_cnt++ ] = assembler->labels[ argument.value ];
//                         break;

//                     case VOID:
//                     case UNKNOWN:
//                     case NUMBER:
//                     case REGISTER:
//                     default:
//                         fprintf( stderr, COLOR_BRIGHT_RED "Incorrect mark for CALL in file: %s:%lu\n", assembler->asm_file.address, i + 1 );
//                         return FAIL_RESULT;
//                 }

//                 PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 2],
//                                                                                   assembler->byte_code[ assembler->instruction_cnt - 1 ] );
//                 break;

//             case RET_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;

//                 PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 1 ] );
//                 break;

//             case HLT_CMD:
//                 assembler->byte_code[ assembler->instruction_cnt++ ] = command;
//                 HLT_flag++;

//                 PRINT( COLOR_BRIGHT_GREEN "%-10s --- %-2d \n", strings[i].ptr, assembler->byte_code[ assembler->instruction_cnt - 1 ] );
//                 break;

//             default:
//                 fprintf( stderr, COLOR_BRIGHT_RED "Incorrect command \"%s\" in file: %s:%lu \n", strings[i].ptr, assembler->asm_file.address, i + 1 );

//                 return FAIL_RESULT;
//         }
//     }

//     if ( HLT_flag ) return SUCCESS_RESULT;

//     fprintf( stderr, COLOR_BRIGHT_RED "There is no HLT command \n" COLOR_RESET );
//     return FAIL_RESULT;
// }

// int AsmCodeProcessing( char* instruction ) {
//     my_assert( instruction, ASSERT_ERR_NULL_PTR );

//     if ( instruction[0] == ':' )                  { return MARK_CMD; }

//     if ( StrCompare( instruction, "PUSH" ) == 0 ) { return PUSH_CMD; }          // TODO: use macros
//     if ( StrCompare( instruction, "POP"  ) == 0 ) { return POP_CMD;  }
//     if ( StrCompare( instruction, "ADD"  ) == 0 ) { return ADD_CMD;  }
//     if ( StrCompare( instruction, "SUB"  ) == 0 ) { return SUB_CMD;  }
//     if ( StrCompare( instruction, "MUL"  ) == 0 ) { return MUL_CMD;  }
//     if ( StrCompare( instruction, "DIV"  ) == 0 ) { return DIV_CMD;  }
//     if ( StrCompare( instruction, "POW"  ) == 0 ) { return POW_CMD;  }
//     if ( StrCompare( instruction, "SQRT" ) == 0 ) { return SQRT_CMD; }
//     if ( StrCompare( instruction, "IN"   ) == 0 ) { return IN_CMD;   }
//     if ( StrCompare( instruction, "OUT"  ) == 0 ) { return OUT_CMD;  }
//     if ( StrCompare( instruction, "CALL" ) == 0 ) { return CALL_CMD; }
//     if ( StrCompare( instruction, "RET"  ) == 0 ) { return RET_CMD;  }

//     if ( StrCompare( instruction, "JMP"  ) == 0 ) { return JMP_CMD;  }
//     if ( StrCompare( instruction, "JE"   ) == 0 ) { return JE_CMD;   }
//     if ( StrCompare( instruction, "JB"   ) == 0 ) { return JB_CMD;   }
//     if ( StrCompare( instruction, "JA"   ) == 0 ) { return JA_CMD;   }
//     if ( StrCompare( instruction, "JBE"  ) == 0 ) { return JBE_CMD;  }
//     if ( StrCompare( instruction, "JAE"  ) == 0 ) { return JAE_CMD;  }

//     if ( StrCompare( instruction, "HLT"  ) == 0 ) { return HLT_CMD;  }

//     return FAIL_RESULT;
// }

void OutputInFile( Assembler_t* assembler ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );

    FILE* file = fopen( assembler->exe_file.address, "w" );
    assert( file && "Failed to open file" );

    fprintf( file, "%lu", assembler->instruction_cnt );

    for ( size_t i = 0; i < assembler->instruction_cnt; i++ ) {
        fprintf( file, " %d", assembler->byte_code[i] );
    }

    int result_of_fclose = fclose( file );
    my_assert( !result_of_fclose, ASSERT_ERR_FAIL_CLOSE )
}

int TranslateAsmToByteCode( Assembler_t* assembler, StrPar* strings ) {
    my_assert( assembler, ASSERT_ERR_NULL_PTR );
    my_assert( strings,   ASSERT_ERR_NULL_PTR );

    assembler->instruction_cnt = 0;

    char* instruction = NULL;
    ArgumentStat argument = {};

    int number_of_params = 0;
    int number_of_characters_read = 0;
    const char* str_pointer = 0;
    int flag_HLT = 0;

    for ( size_t i = 0; i < assembler->asm_file.nLines; i++ ) {
        str_pointer = strings[i].ptr;
        
        // Check if the line starts with a label marker ':'
        if (str_pointer[0] == ':') {
            // Process label
            char label_char = str_pointer[1];
            if (label_char >= '0' && label_char <= '9') {
                int label_idx = label_char - '0';
                assembler->labels[label_idx].instruction_ptr = (int)assembler->instruction_cnt;
                
                PRINT(COLOR_BRIGHT_GREEN "%-10s\\n", strings[i].ptr);
                continue; // Move to next line
            } else {
                fprintf(stderr, "Incorrect label format in file: %s:%lu\\n", assembler->asm_file.address, i + 1);
                return FAIL_RESULT;
            }
        }
        
        number_of_params = sscanf(str_pointer, "%ms%n", &instruction, &number_of_characters_read);
        if (number_of_params == 0) continue;

        str_pointer += number_of_characters_read;
        ArgumentProcessing(assembler, &argument, str_pointer);

        bool command_found = false;
        for (size_t idx = 0; idx < number_of_commands; idx++) {
            if (StrCompare(instruction, commands[idx].command_name) == 0) {
                command_found = true;
                
                // Special handling for MARK command (which represents labels)
                if (commands[idx].command_number == MARK_CMD) {
                    if (argument.type == MARK) {
                        assembler->labels[argument.value].instruction_ptr = (int)assembler->instruction_cnt;
                        
                        PRINT(COLOR_BRIGHT_GREEN "%-10s\\n", strings[i].ptr);
                        break; // Continue to next line
                    } else {
                        fprintf(stderr, "Incorrect mark in file: %s:%lu\\n", assembler->asm_file.address, i + 1);
                        free(instruction);
                        return FAIL_RESULT;
                    }
                }
                
                // Process arguments based on command type
                if (commands[idx].type_of_param != VOID && argument.type != commands[idx].type_of_param) {
                    fprintf(stderr, COLOR_BRIGHT_RED "Incorrect argument type for %s in file: %s:%lu \\n", 
                            instruction, assembler->asm_file.address, i + 1);
                    free(instruction);
                    return FAIL_RESULT;
                }

                assembler->byte_code[assembler->instruction_cnt++] = commands[idx].command_number;

                // Handle special cases for commands that need argument processing
                if (commands[idx].type_of_param != VOID) {
                    if (commands[idx].type_of_param == LABEL && argument.type == MARK) {
                        // For jump commands with label arguments
                        assembler->byte_code[assembler->instruction_cnt++] = assembler->labels[argument.value].instruction_ptr;
                    } else {
                        // Regular argument handling
                        assembler->byte_code[assembler->instruction_cnt++] = argument.value;
                    }
                }

                // Check if it's HLT command
                if (commands[idx].command_number == HLT_CMD) {
                    flag_HLT++;
                }

                // Print debug info
                if (commands[idx].type_of_param != VOID) {
                    PRINT(COLOR_BRIGHT_GREEN "%-10s --- %-2d %d \\n", strings[i].ptr, 
                          assembler->byte_code[assembler->instruction_cnt - 2],
                          assembler->byte_code[assembler->instruction_cnt - 1]);
                } else {
                    PRINT(COLOR_BRIGHT_GREEN "%-10s --- %-2d \\n", strings[i].ptr, 
                          assembler->byte_code[assembler->instruction_cnt - 1]);
                }

                break; // Command processed, move to next line
            }
        }

        if (!command_found) {
            fprintf(stderr, COLOR_BRIGHT_RED "Unknown command \"%s\" in file: %s:%lu \\n", 
                    instruction, assembler->asm_file.address, i + 1);
            free(instruction);
            return FAIL_RESULT;
        }

        free(instruction); // Free the allocated instruction string
    }

    if (flag_HLT) return SUCCESS_RESULT;

    fprintf(stderr, COLOR_BRIGHT_RED "There is no HLT command \\n" COLOR_RESET);
    return FAIL_RESULT;
}

int ArgumentProcessing( Assembler_t* assembler, ArgumentStat* argument, const char* string ) {
    my_assert( argument, ASSERT_ERR_NULL_PTR );
    my_assert( string,   ASSERT_ERR_NULL_PTR );

    int number_of_elements_read = 0;
    argument->type = VOID;
    argument->value = 0;

    number_of_elements_read = sscanf( string, "%d", &( argument->value ) );
    if ( number_of_elements_read == 1 ) {
        argument->type = NUMBER;

        return argument->value;
    }

    char* instruction = NULL;
    number_of_elements_read = sscanf( string, "%ms", &instruction );
    if ( number_of_elements_read == 1 ) {
        if ( instruction[0] == ':' ) {
            argument->value = instruction[1] - '0';

            if ( argument->value >= 0 && argument->value < 10 ) {       // magic number
                argument->type = MARK;

                return argument->value;
            }
        }

        if ( strlen( instruction ) == 3 && instruction[0] == 'R' && instruction[2] == 'X' ) {   // magic number | massive with registers
            argument->value = instruction[1] - 'A' + 1;

            if ( argument->value < 8 ) {
                argument->type = REGISTER;

                return argument->value;
            }
        }
    }
    else {
        argument->type = VOID;
        return argument->value;
    }

    PRINT( "%s\n", instruction );
    argument->type = UNKNOWN;
    argument->value = -1;
    return argument->value;
}





#ifdef _DEBUG
void PrintLabels( int labels[ LABELS_NUMBER ] ) {
    my_assert( labels, ASSERT_ERR_NULL_PTR );

    PRINT( COLOR_BLUE "Labels: \n" );
    for ( size_t i = 0; i < LABELS_NUMBER; i++ ) PRINT( COLOR_BRIGHT_CYAN "%-2lu - %d \n", i, labels[ i ] );
}
#endif