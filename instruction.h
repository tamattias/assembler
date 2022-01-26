#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#define INSTRUCTION_OPCODE(inst) ((inst) & 0xFFFF) 
#define INSTRUCTION_FUNCT(inst)  ((inst) >> 16)
#define MAKE_INSTRUCTION(opcode, funct) (((opcode) & 0xFFFF) | ((funct) << 16))

typedef int instruction_t;

instruction_t find_instruction(const char *name);

#endif
