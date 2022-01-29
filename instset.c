/**
 * @file instruction.c
 * @author Tamir Attias
 * @file Instruction set implementations.
 */

#include "instset.h"

#include <string.h>

static const inst_desc_t instruction_set[] = {
    {"mov",  INST_MOV,  2, {ADDR_MODE_ALL, ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"cmp",  INST_CMP,  2, {ADDR_MODE_ALL, ADDR_MODE_ALL}},
    {"add",  INST_ADD,  2, {ADDR_MODE_ALL, ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"sub",  INST_SUB,  2, {ADDR_MODE_ALL, ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"lea",  INST_LEA,  2, {ADDR_MODE_DIRECT | ADDR_MODE_INDEX, ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"clr",  INST_CLR,  1, {ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"not",  INST_NOT,  1, {ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"inc",  INST_INC,  1, {ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"dec",  INST_DEC,  1, {ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"jmp",  INST_JMP,  1, {ADDR_MODE_DIRECT | ADDR_MODE_INDEX}},
    {"bne",  INST_BNE,  1, {ADDR_MODE_DIRECT | ADDR_MODE_INDEX}},
    {"jsr",  INST_JSR,  1, {ADDR_MODE_DIRECT | ADDR_MODE_INDEX}},
    {"red",  INST_RED,  1, {ADDR_MODE_ALL & ~ADDR_MODE_IMMEDIATE}},
    {"prn",  INST_PRN,  1, {ADDR_MODE_ALL}},
    {"rts",  INST_RTS,  0},
    {"stop", INST_STOP, 0},
};

static const int instruction_set_size = sizeof(instruction_set) / sizeof(instruction_set[0]);

const inst_desc_t *find_inst(const char *mne)
{
    int i;
    for (i = 0; i < instruction_set_size; ++i) {
        if (strcmp(instruction_set[i].mne, mne) == 0)
            return &instruction_set[i];
    }
    return 0;
}
