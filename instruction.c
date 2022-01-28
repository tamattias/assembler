/**
 * @file instruction.c
 * @author Tamir Attias
 * @file Instruction set implementations.
 */

#include "instruction.h"

#include <string.h>

/**
 * Mapping of instruction mnemonics to instructions.
 */
typedef struct {
    const char *mne;
    const inst_t instruction;
} inst_desc_t;

static const inst_desc_t instruction_set[] = {
    {"mov", INST_MOV},
    {"cmp", INST_CMP},
    {"add", INST_ADD},
    {"sub", INST_SUB},
    {"lea", INST_LEA},
    {"clr", INST_CLR},
    {"not", INST_NOT},
    {"inc", INST_INC},
    {"dec", INST_DEC},
    {"jmp", INST_JMP},
    {"bne", INST_BNE},
    {"jsr", INST_JSR},
    {"red", INST_RED},
    {"prn", INST_PRN},
    {"rts", INST_RTS},
    {"stop", INST_STOP},
};

static const int instruction_set_size = sizeof(instruction_set) / sizeof(instruction_set[0]);

inst_t find_inst(const char *mne)
{
    int i;
    for (i = 0; i < instruction_set_size; ++i) {
        if (strcmp(instruction_set[i].mne, mne) == 0)
            return instruction_set[i].instruction;
    }
    return INST_BAD;
}
