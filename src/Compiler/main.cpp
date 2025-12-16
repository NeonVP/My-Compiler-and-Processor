#include "Compiler.h"


int main( int argc, char** argv ) {
    Assembler_t assembler = {};

    AssemblerCtor( &assembler, argc, argv );

    PRINT( COLOR_BRIGHT_WHITE "FILES:\nfor input - %s\nfor output - %s\n", assembler.asm_file.address, assembler.exe_file.address );

    int processing_result = AsmCodeToByteCode( &assembler );

    if ( processing_result != 1 ) {
        fprintf( stderr, COLOR_BRIGHT_RED "Emergency shutdown of the assembler \n" );
        AssemblerDtor( &assembler );

        return 1;
    }

    OutputInFile( &assembler );

    AssemblerDtor( &assembler );
    return 0;
}