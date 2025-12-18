#ifndef COMMON_H
#define COMMON_H

enum ASM_CMD {
    PUSH_CMD  = 1,
    POP_CMD   = 2,
    ADD_CMD   = 3,
    SUB_CMD   = 4,
    MUL_CMD   = 5,
    DIV_CMD   = 6,
    POW_CMD   = 7,
    SQRT_CMD  = 8,
    IN_CMD    = 9,
    OUT_CMD   = 10,
    JMP_CMD   = 11,
    JB_CMD    = 12,
    JA_CMD    = 13,
    JBE_CMD   = 14,
    JAE_CMD   = 15,
    JE_CMD    = 16,
    HLT_CMD   = 17,
    CALL_CMD  = 28,
    RET_CMD   = 29,
    MARK_CMD  = 30,
    PUSHR_CMD = 33,
    POPR_CMD  = 34,
    PUSHM_CMD = 35,  // Цитирование из RAM по адресу в регистре
    POPM_CMD  = 36   // Запись в RAM по адресу в регистре
};

// struct Commands_t {
//     int command_number = 0;
//     void* function_pointer = NULL;
//     int number_of_parameters = 0;
// } commands[] = {

// };

#endif