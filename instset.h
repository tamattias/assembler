/**
 * @file instset.h
 * @author Tamir Attias
 * @brief Instruction set declarations.
 */

#ifndef INSTSET_H
#define INSTSET_H

/**
 * Pack an opcode and funct to a single value.
 */
#define MAKE_INST(opcode, funct) ((inst_t)((opcode) & 0xF) | (inst_t)((funct) << 4))

/**
 * Extracts opcode from instruction.
 */
#define INST_OPCODE(inst) ((inst) & 0xF) 

/**
 * Extract function code from instruction.
 */
#define INST_FUNCT(inst)  ((inst) >> 4)

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

/**
 * Make the first code word for an encoded instruction.
 */
#define MAKE_FIRST_INST_WORD(opcode, e, r, a) \
    ( \
        ((word_t)1 << (opcode))       | \
        ((word_t)(e) ? (1 << 16) : 0) | \
        ((word_t)(r) ? (1 << 17) : 0) | \
        ((word_t)(a) ? (1 << 18) : 0)   \
    )

/**
 * Make the second code word for an encoded instruction.
 */
#define MAKE_SECOND_INST_WORD(dst_addr_mode, dst_reg, src_addr_mode, src_reg, funct, e, r, a) \
    ( \
        ((word_t)(dst_addr_mode) & 0x3)        | \
        (((word_t)(dst_reg) & 0xFF) << 2)      | \
        (((word_t)(src_addr_mode) & 0x3) << 6) | \
        (((word_t)(src_reg) & 0xFF) << 8)      | \
        (((word_t)(funct) & 0xFF) << 12)       | \
        ((word_t)(e) ? (1 << 16) : 0)          | \
        ((word_t)(r) ? (1 << 17) : 0)          | \
        ((word_t)(a) ? (1 << 18) : 0)           \
    )

/**
 * Make an extra code word for an encoded instruction.
 */
#define MAKE_EXTRA_INST_WORD(value, e, r, a) \
    ( \
        (((word_t)(value) & 0xFFFF))   | \
        ((word_t)(e) ? (1 << 16) : 0)  | \
        ((word_t)(r) ? (1 << 17) : 0)  | \
        ((word_t)(a) ? (1 << 18) : 0)    \
    )

/**
 * Makes a data segment word.
 */
#define MAKE_DATA_WORD(datum) (((word_t)(datum) & 0xFFFF) | (1 << 18))

/**
 * Maximum number of operands per instruction.
 */
#define MAX_OPERANDS 2

/**
 * Instruction code.
 */
typedef unsigned char inst_t;

/**
 * Data type used internally for representing a machine word.
 * We use a long because it is guaranteed to be at least 32 bits wide.
 */
typedef long word_t;

/**
 * Addressing mode.
 */
typedef enum {
    ADDR_MODE_IMMEDIATE = (1 << 0),
    ADDR_MODE_DIRECT = (1 << 1),
    ADDR_MODE_INDEX = (1 << 2),
    ADDR_MODE_REGISTER_DIRECT = (1 << 3)
} addr_mode_t;

#define ADDR_MODE_ALL (ADDR_MODE_IMMEDIATE | ADDR_MODE_DIRECT | ADDR_MODE_INDEX | ADDR_MODE_REGISTER_DIRECT)

/**
 * Describes the structure of a valid instruction.
 */
typedef struct {
    /** Mnemonic. */
    const char *mne;
    /** Instruction code. */
    const inst_t instruction;
    /** Number of operands. */
    int noperands;
    /** Legal addressing modes for each operand (bitfield of ADDR_MODE_*). */
    int addr_modes[MAX_OPERANDS];
} inst_desc_t;

/**
 * Finds the description of an instruction by its mnemonic.
 *
 * @param mne Instruction mnemonic.
 * @return Pointer to instruction description or null if no such instruction.
 */
const inst_desc_t *find_inst(const char *mne);

#endif
