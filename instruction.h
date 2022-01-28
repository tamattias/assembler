/**
 * @file instruction.h
 * @author Tamir Attias
 * @brief Instruction set declarations.
 */

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

/**
 * Make a complete instruction ID from opcode and function code.
 */
#define MAKE_INST(opcode, funct) (((opcode) & 0xFFFF) | ((funct) << 16))

/**
 * Extracts opcode from instruction.
 */
#define INST_OPCODE(inst) ((inst) & 0xFFFF) 

/**
 * Extract function code from instruction.
 */
#define INST_FUNC(inst)  ((inst) >> 16)

#define INST_BAD (-1) /** Bad instruction. */
#define INST_MOV  MAKE_INST(0, 0)
#define INST_CMP  MAKE_INST(1, 0)
#define INST_ADD  MAKE_INST(2, 10)
#define INST_SUB  MAKE_INST(2, 11)
#define INST_LEA  MAKE_INST(4, 0)
#define INST_CLR  MAKE_INST(5, 10)
#define INST_NOT  MAKE_INST(5, 11)
#define INST_INC  MAKE_INST(5, 12)
#define INST_DEC  MAKE_INST(5, 13)
#define INST_JMP  MAKE_INST(9, 10)
#define INST_BNE  MAKE_INST(9, 11)
#define INST_JSR  MAKE_INST(9, 12)
#define INST_RED  MAKE_INST(12, 0)
#define INST_PRN  MAKE_INST(13, 0)
#define INST_RTS  MAKE_INST(14, 0)
#define INST_STOP MAKE_INST(15, 0)

typedef int inst_t;

/**
 * Translates an instruction mnemonic to an instruction.
 *
 * @param mne Instruction mnemonic.
 * @return The instruction (one of INST_) or INST_BAD.
 */
inst_t find_inst(const char *mne);

#endif
