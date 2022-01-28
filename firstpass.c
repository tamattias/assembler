/**
 * @file firstpass.c
 * @author Tamir Attias
 * @brief First pass implementation.
 */

#include "firstpass.h"
#include "constants.h"
#include "ioutil.h"
#include "shared.h"
#include "instruction.h"
#include "symtable.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* TODO: Handle cases of memory image overflow (data/instruction). */

/**
 * First pass internal state.
 */
typedef struct {
    int ic; /** Instruction counter. */
    int dc; /** Data counter. */
    int line_no; /** Current line number. */
    char *line_head; /** Line head. */
    int eol; /** End of line reached? */
    char field[MAX_LINE_LENGTH + 1]; /** Current field. */
    int field_len; /** Length of current field. */
    int labeled; /** Is current line a label line? */
    char label[MAX_LABEL_LENGTH + 1]; /** Current label. */
    int label_len; /** Length of current label. */
} firstpass_t;

static void report_error(firstpass_t *fp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("error: line %d: ", fp->line_no);
    vprintf(fmt, args);
    putchar('\n'); /* Newline at end. */
    va_end(args);
}

/**
 * Parses a comma separated list of integer values passed to a data directive.
 *
 * @param input Pointer to a null terminated string containing a comma
 *              separated list of integers.
 * @param data Pointer to an array of words in which to store the read
 *             data.
 * @param limit Maximum number of items that can be read into data.
 * @note Only the first MAX_LINE_LENGTH bytes of data will be scanned.
 * @return Number of integers read or -1 if invalid data.
 */
static int read_comma_separated_data(const char *input, word_t *data, int limit)
{
    char tmpstr[MAX_LINE_LENGTH + 1]; /* Copy of input for tokenization. */
    char *tok; /* Current token. */
    word_t nval; /* Integer parsed from current token. */
    int count = 0; /* Tokens read successfully so far. */
    
    /* Make a copy of the input for tokenization. */
    strncpy(tmpstr, input, MAX_LINE_LENGTH);

    /* Begin tokenization. */
    tok = strtok(tmpstr, ",");
    while (tok) {
        /* Read integer from token. */
        /* TODO: Make sure this only allows decimal numbers (not hex etc.) */
        /* TODO: Don't allow whitespace separated values between commas. */
        /* TODO: Use parse_number instead? */
        if (sscanf(tok, "%ld", &nval) != 1)
            return -1; /* Not integer, abort. */

        /* Store in output array. */
        data[count++] = nval;

        /* Check if we read too many items. */
        if (count > limit)
            break;

        /* Get next token. */
        tok = strtok(NULL, ",");
    }
    
    return count;
}

static int process_label_field(firstpass_t *fp, shared_t *shared)
{
    int bad = 0; /* Label badly formatted? */
    const char *head = fp->field;

    while ((fp->label[fp->label_len++] = *head++) != ':') {
        /* Check if symbol too long. */
        if (fp->label_len > MAX_LABEL_LENGTH) {
            report_error(fp, "label is too long (%d chars, max. %d).", fp->label_len, MAX_LABEL_LENGTH);
            bad = 1; /* Flag symbol as bad. */
            break;
        }

        /* Check if alphanumeric. */
        if (!isalnum(fp->label[fp->label_len - 1])) {
            report_error(fp, "invalid character '%c' in label (only alphanumeric characters allowed)",
                fp->label[fp->label_len - 1]);
            bad = 1; /* Flag symbol as bad. */
            break;
        }
    }

    if (bad)
        return 1;

    /* Check if symbol empty. */
    if (fp->label_len <= 0) {
        report_error(fp, "label is empty.");
        return 1;
    }

    if (symtable_find(shared->symtable, fp->label)) {
        report_error(fp, "label %s already defined; ignoring statement.", fp->label);
        return 1;
    }

    return 0;
}

/**
 * Reads the next field and sets the eol flag if end of line reached.

 * @return Non-zero if end of line, else zero.
 */
static int get_next_field(firstpass_t *fp)
{
    read_field(&fp->line_head, fp->field, &fp->field_len);
    return is_eol(*fp->line_head);
}

/**
 * Process data after a .data directive.
 */
static int process_data_directive(firstpass_t *fp, shared_t *shared)
{
    int data_len; /* Number of words read. */
    symbol_t *sym; /* Symbol. */

    if (fp->eol) {
        report_error(fp, "missing data after data directive.");
        return 1;
    }
    
    /* Read comma separated integer values into data image. */
    data_len = read_comma_separated_data(
        fp->line_head,
        shared->data_image + fp->dc,
        MAX_DATA_LENGTH);

    /* Check if bad data. */
    if (data_len == -1) {
        report_error(fp, "invalid data after data directive.");
        return 1;
    }

    /* Check if empty data. */
    if (data_len == 0) {
        report_error(fp, "no data after data directive.");
        return 1;
    }

    /* Add symbol if labeled. */
    if (fp->labeled) {
        sym = symtable_new(shared->symtable, fp->label);
        assert(sym);
        sym->data = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(fp->dc);
        sym->offset = SYMBOL_OFFSET(fp->dc);
    }

    /* Increment data counter. */
    fp->dc += data_len;

    return 0;
}

/**
 * Process data after a .string directive.
 */
static int process_string_directive(firstpass_t *fp, shared_t *shared)
{
    char c; /* Last read string directive character. */
    int len; /* Number of characters read. */

    if (fp->eol) {
        report_error(fp, "missing string data after string directive.");
        return 1;
    }

    /* Skip whitespace. */
    while ((c = *fp->line_head++) != '\0' && isspace(c))
        ;

    /* If last character was end of line then no data is present. */
    if (is_eol(c)) {
        report_error(fp, "missing string data after string directive.");
        return 1;
    }

    /* Unread last character. */
    --fp->line_head;

    /* String needs to begin with double quotes. */
    if (*fp->line_head++ != '"') {
        report_error(fp, "string data missing opening double quotes.");
        return 1;
    }

    len = 0;

    /* Copy string into data image, incrementing the data counter for
       every character (word) written. */
    while ((c = *fp->line_head++) != '\0' && c != '"')
        shared->data_image[fp->dc++] = c;

    /* Check if string is improperly terminated. */
    if (c != '"') {
        report_error(fp, "string data missing closing quotes.");
        return 1;
    }

    /* Append null terminator. */
    shared->data_image[fp->dc++] = '\0';

    /* Increment data counter by string length plus null terminator length. */
    fp->dc += len * sizeof(int) + 1;

    return 0;
}

/**
 * parse_operand return values.
 */
typedef enum {
    PARSE_OPERAND_OK,   /** Operand parsed successfully. */
    PARSE_OPERAND_BAD,  /** Token was not a valid operand. */
    PARSE_OPERAND_EMPTY /** Token was blank. */
} parse_operand_result_t;

static parse_operand_result_t parse_operand(firstpass_t *fp, const char *tok, operand_t *op)
{
    char c; /* Current character. */
    int label_len; /* Label length. */
    int reg_id; /* Register ID. */

    /* Skip whitespace. */
    while (!is_eol(c = *tok++) && isspace(c))
        ;

    /* Check if empty string. */
    if (is_eol(c))
        return PARSE_OPERAND_EMPTY;

    /* Unread non-whitespace char. */
    --tok;

    /* Check if immediate mode. */
    if (tok[0] == '#') {
        /* Apply address mode. */
        op->addr_mode = ADDR_MODE_IMMEDIATE;

        /* Parse immediate value. */
        if (parse_number(tok + 1, &op->value.immediate) != 0) {
            report_error(fp, "could not parse immediate number in operand.");
            return PARSE_OPERAND_BAD;
        }

        return PARSE_OPERAND_OK;
    }

    /* Check if direct register mode. */
    if (tok[0] == 'r' && parse_number(tok + 1, &op->value.reg) == 0) {
        /* Apply address mode. */
        op->addr_mode = ADDR_MODE_REGISTER_DIRECT;

        return PARSE_OPERAND_OK;
    }

    /* This can be either direct mode or index mode. */

    /* Initialize label length. */
    label_len = 0;

    /* Read the label. */
    while (!is_eol(c = *tok++) && isalnum(c)) {
        /* Check if label is too long. */
        if (label_len >= MAX_LABEL_LENGTH) {
            report_error(fp, "label too long.");
            return PARSE_OPERAND_BAD;
        }

        /* Append read character. */
        op->label[label_len++] = c;
    }

    /* Unread last character. */
    --tok;

    /* Check if end of label. */
    if (is_eol(c)) {
        /* Check if empty label. */
        if (label_len == 0) {
            report_error(fp, "label is empty.");
            return PARSE_OPERAND_BAD;
        }

        /* Apply address mode. */
        op->addr_mode = ADDR_MODE_DIRECT;

        /* Append null terminator. */
        op->label[label_len] = 0;
           
        return PARSE_OPERAND_OK;
    }

    if (!isspace(c) && c != '[') {
        /* Found non-alphanumeric character in label. */
        report_error(fp, "invalid label (non-alphanumeric character: \'%c\').\n", c);
        return PARSE_OPERAND_BAD;
    }

    /* Check for extraneous characters. */
    while (!is_eol(c = *tok++) && isspace(c))
        ;
    if (!is_eol(c) && c != '[') {
        /* Found extraneous character. */
        report_error(fp, "direct addressing operand has extraneous characters.");
        return PARSE_OPERAND_BAD;
    }

    /* Unread last character. */
    --tok;

    /* Check if we have a register subscript. */
    if (c == '[') {
        /* Read register index. */
        if (sscanf(tok, "[r%d]", &reg_id) != 1) {
            report_error(fp, "could not read register value from brackets.");
            return PARSE_OPERAND_BAD;
        }

        /* Check if offset register is valid. */
        if (reg_id < 0 || reg_id > 15) {
            report_error(fp, "register value out of range: %d (must be between 0 and 15)\n", (int)op->value.reg);
            return PARSE_OPERAND_BAD;
        }

        /* Store register ID in operand. */
        op->value.reg = (word_t)reg_id;

        /* Apply address mode. */
        op->addr_mode = ADDR_MODE_INDEX;

        return PARSE_OPERAND_OK;
    }

    /* Apply direct mode. */
    op->addr_mode = ADDR_MODE_DIRECT;

    /* Null terminate label. */
    op->label[label_len] = '\0';

    return PARSE_OPERAND_OK;
}

/**
 * Process operands beginning at the head pointer.
 *
 * @note This destroys the line buffer so the line should not be
 *       accessed afterward.
 * @return Number of operands read or -1 on failure.
 */
static int process_operands(firstpass_t *fp, operand_t ops[])
{
    char *tok; /* Token. */
    int nops = 0; /* Number of operands. */
    int parse_result; /* Result returned from parse_operand. */

    /* First token. */
    tok = strtok(fp->line_head, ",");

    while (tok) {
        /* Check if too many operands. */
        if (nops >= MAX_OPERANDS) {
            report_error(fp, "too many operands.");
            return -1;
        }

        /* Parse operand from token. */
        parse_result = parse_operand(fp, tok, &ops[nops]);
        
        /* Check if invalid operand. */
        if (parse_result == PARSE_OPERAND_BAD)
            return -1;

        /* See if there is another token. */
        tok = strtok(NULL, ",");

        if (parse_result == PARSE_OPERAND_EMPTY) {
            /* If there is another operand but this one was empty then this is
               invalid. Otherwise, the operand list is empty which is fine. */
            return tok ? -1 : 0; 
        }

        /* Increment only after making sure the operand isn't empty. */
        nops++;
    }

    return nops;
}

#if 0
static void debug_print_operand(operand_t *op)
{
    switch (op->addr_mode) {
    case ADDR_MODE_IMMEDIATE:
        printf("Immediate operand: immediate=%d\n", (int)op->value.immediate);
        break;
    case ADDR_MODE_REGISTER_DIRECT:
        printf("Register Direct operand: reg=r%d\n", (int)op->value.reg);
        break;
    case ADDR_MODE_DIRECT:
        printf("Direct operand: label=%s\n", op->label);
        break;
    case ADDR_MODE_INDEX:
        printf("Index operand label=%s register=r%d\n", op->label, (int)op->value.reg);
        break;
    }
}
#endif

static int addr_mode_to_index(addr_mode_t addr_mode) {
    switch (addr_mode) {
    case ADDR_MODE_IMMEDIATE:
        return 0;

    case ADDR_MODE_DIRECT:
        return 1;

    case ADDR_MODE_INDEX:
        return 2;

    case ADDR_MODE_REGISTER_DIRECT:
        return 3;
    }
    return 0;
}

static int write_extra_words(firstpass_t *fp, shared_t *shared, const operand_t *op)
{
    switch (op->addr_mode) {
    case ADDR_MODE_IMMEDIATE: 
        /* Write extra word containing the immediate value with A flag set. */
        shared->code_image[fp->ic++] = MAKE_EXTRA_INST_WORD(
            op->value.immediate, /* Value */
            0, /* E flag */
            0, /* R flag */
            1  /* A flag */
        );
        break;
    
    case ADDR_MODE_DIRECT:
        /* Preserve space for two extra words, filled later in second pass. */
        fp->ic += 2;
        break;

    case ADDR_MODE_INDEX:
        /* Preserve space for two extra words, filled later in second pass. */
        fp->ic += 2;
        break;

    case ADDR_MODE_REGISTER_DIRECT:
        /* No extra words needed, register stored in second word. */
        break;
    }
    return 0;
}

/**
 * Process an instruction.
 */
static int process_instruction(firstpass_t *fp, shared_t *shared)
{
    const inst_desc_t *desc; /* Instruction code. */
    operand_t ops[MAX_OPERANDS]; /* Operands. */
    int nops; /* Number of operands. */
    symbol_t *sym; /* Symbol. */
    word_t addr; /* Address of instruction. */
    int i; /* Counter. */
    int src_reg, dst_reg; /* Source and destination register numbers for encoding second instruction. */
    
    /* Look up instruction name by mnemonic. */
    desc = find_inst(fp->field);

    /* No such mnemonic. */
    if (!desc) {
        report_error(fp, "bad instruction mnemonic: %s", fp->field);
        return 1;
    }

    /* Parse operands. */
    if ((nops = process_operands(fp, ops)) < 0)
        return 1;

    if (desc->noperands != nops) {
        report_error(fp, "incorrect number of operands (expected %d, got %d)", desc->noperands, nops);
        return 1;
    }

    /* Store instruction address before incrementing IC. */
    addr = fp->ic;

    /* Write first word (opcode). */
    shared->code_image[fp->ic++] = MAKE_FIRST_INST_WORD(INST_OPCODE(desc->instruction), 0, 0, 0);

    if (nops > 0) {
        for (i = 0; i < nops; ++i) {
            /* Check if the addressing mode used is legal by checking if its bit
            is set in the addressing modes bitfield of the description. */
            if (!(desc->addr_modes[i] & ops[i].addr_mode)) {
                report_error(fp, "operand %d has invalid addressing mode.", i + 1);
                return 1;
            }
        }

        /* Default to zeroth register. */
        src_reg = 0;
        dst_reg = 0;

        /**
         * Determine register value based on operands.
         */ 
        if (nops == 1) {
            /* Check if destination operand has index or direct addressing mode. */
            if (ops[0].addr_mode & (ADDR_MODE_INDEX | ADDR_MODE_REGISTER_DIRECT)) {
                src_reg = 0;
                dst_reg = ops[0].value.reg;
            }
        } else {
            /* Check if source operand has index or direct addressing mode. */
            if (ops[0].addr_mode & (ADDR_MODE_INDEX | ADDR_MODE_REGISTER_DIRECT)) {
                src_reg = ops[0].value.reg;
            }

            /* Check if destination operand has index or direct addressing mode. */
            if (ops[1].addr_mode & (ADDR_MODE_INDEX | ADDR_MODE_REGISTER_DIRECT)) {
                dst_reg = ops[1].value.reg;
            }
        }

        /*
         * Write the second word which contains the function code as well as
         * addressing modes for the operands.
         *
         * The addressing modes for the source and destination registers as
         * well as the register numbers are determined based on the number of
         * operands. If there is a single operand then it is the destination
         * operand and so the source addressing mode and register number will
         * be set to zero. Otherwise, the source addressing mode and register
         * number will be deteremined by the first operand and the destination
         * addressing mode and register number will be determined by the
         * second operand.
         */
        shared->code_image[fp->ic++] = MAKE_SECOND_INST_WORD(
            nops == 1 ? addr_mode_to_index(ops[0].addr_mode) : addr_mode_to_index(ops[1].addr_mode), /* dst addr mode */
            dst_reg, /* dst register */
            nops == 2 ? addr_mode_to_index(ops[0].addr_mode) : 0, /* src addr mode */
            src_reg, /* src register */
            INST_FUNCT(desc->instruction), /* funct */
            0, /* E flag */
            0, /* R flag */
            0  /* A flag */
        );

        /* Write/preserve extra words for first operand. */
        write_extra_words(fp, shared, &ops[0]);

        /* Write/preserve extra words for second operand if specified. */
        if (nops > 1)
            write_extra_words(fp, shared, &ops[1]);
    }

     if (fp->labeled) {
        /* Make a symbol for the instruction. */
        sym = symtable_new(shared->symtable, fp->label);
        assert(sym);
        sym->code = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(addr);
        sym->offset = SYMBOL_OFFSET(addr);
    }

    return 0;
}

static int process_line(firstpass_t *fp, char *line, shared_t *shared)
{
    symbol_t *sym;

    /* Increment line counter. */
    ++fp->line_no;

    /* Initialize head to beginning of line. */
    fp->line_head = line;

    /* First field. */
    if (get_next_field(fp) != 0)
        return 0; /* Skip empty line. */

    /* Handle comment lines. */
    if (fp->field[0] == ';')
        return 0; /* Skip comment line. */

    /* Check if first field is a label. */
    if (fp->field[fp->field_len - 1] == ':') {
        /* Try to read as label. */
        if (process_label_field(fp, shared) != 0)
            return 1;

        /* Labeled. */
        fp->labeled = 1;

        /* Next field. */
        if (get_next_field(fp) != 0)
            return 0;
    } else {
        /* Not labeled. */
        fp->labeled = 0;
    }

    if (fp->field[0] == '.') {
        /* Process directives. */
        if (strcmp(fp->field + 1, "data") == 0) {
            /* Data directive. */
            if (process_data_directive(fp, shared))
                return 1;
        } else if (strcmp(fp->field + 1, "string") == 0) {
            /* String directive. */
            if (process_string_directive(fp, shared))
                return 1;
        } else if (strcmp(fp->field + 1, "extern") == 0) {
            /* Insert a symbol with external flag  and address and offset
               set to zero. */
            sym = symtable_new(shared->symtable, fp->label);
            assert(sym);
            sym->ext = 1;
            sym->base_addr = 0;
            sym->offset = 0;
        } else if (strcmp(fp->field + 1, "entry") == 0) {
            /* Entry directives ignored in first pass. */
        } 
    } else if (!fp->eol) {
        /* Not a directive, expect instruction. */
        if (process_instruction(fp, shared) != 0)
            return 1;
    } else {
        /* End of line after first field, must be empty label. */
        assert(fp->labeled);

        /* Try to allocate a symbol. */
        sym = symtable_new(shared->symtable, fp->label);
        assert(sym);
        sym->code = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(fp->ic);
        sym->offset = SYMBOL_OFFSET(fp->ic);
    }

    return 0;
}

int firstpass(const char *filename, struct shared *shared)
{
    FILE *in; /* Input file pointer. */
    char line[MAX_LINE_LENGTH + 1]; /* Line buffer. */
    firstpass_t fp; /* Internal state. */
    int error = 0; /* Error flag. */

    /* Try to open input file. */
    if ((in = fopen(filename, "r")) == 0) {
        printf("error: firstpass: could not open input file %s.\n", filename);
        return 1;
    }

    /* Zero initialize internal state. */
    memset(&fp, 0, sizeof(fp));

    /* Process file line by line. */
    while (fgets(line, sizeof(line), in))
        error |= process_line(&fp, line, shared);

    /* Close input file. */
    fclose(in);

    return error;
}
