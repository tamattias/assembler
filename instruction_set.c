#include "instruction_set.h"

#include <string.h>

typedef struct {
    const char *name;
    instruction_t instruction;
} instruction_desc_t;

static instruction_desc_t descs[] = {
    {"mov", MAKE_INSTRUCTION(0, 0)},
    {"cmp", MAKE_INSTRUCTION(1, 0)},
    {"add", MAKE_INSTRUCTION(2, 10)},
    {"sub", MAKE_INSTRUCTION(2, 11)},
    {"lea", MAKE_INSTRUCTION(4, 0)},
    {"clr", MAKE_INSTRUCTION(5, 10)},
    {"not", MAKE_INSTRUCTION(5, 11)},
    {"inc", MAKE_INSTRUCTION(5, 12)},
    {"dec", MAKE_INSTRUCTION(5, 13)},
    {"jmp", MAKE_INSTRUCTION(9, 10)},
    {"bne", MAKE_INSTRUCTION(9, 11)},
    {"jsr", MAKE_INSTRUCTION(9, 12)},
    {"red", MAKE_INSTRUCTION(12, 0)},
    {"prn", MAKE_INSTRUCTION(13, 0)},
    {"rts", MAKE_INSTRUCTION(14, 0)},
    {"stop", MAKE_INSTRUCTION(15, 0)},
};

instruction_t is_find(const char *name)
{
    int i;
    for (i = 0; i < sizeof(descs)/sizeof(descs[0]); ++i) {
        if (strcmp(descs[i].name, name) == 0)
            return descs[i].instruction;
    }
    return 0;
}
