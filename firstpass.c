/**
 * @file firstpass.c
 * @author Tamir Attias
 * @brief First pass implementation.
 */

#include "firstpass.h"
#include "constants.h"
#include "ioutil.h"
#include "shared.h"
#include "instset.h"
#include "symtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* TODO: Handle cases of segment overflow (data/instruction). */

/**
 * Node in a linked list of data symbols.
 */
typedef struct datasym {
    /** Symbol. */
    symbol_t *sym;
    /** Next node. */
    struct datasym *next;
} datasym_t;

/**
 * Internal state for first pass.
 */
typedef struct {
    /** Instruction counter. */
    int ic;
    /** Data counter. */
    int dc;
    /** Current line number. */
    int line_no;
    /** Pointer to next character to process. */
    char *line_head;
    /** Last read field. */
    char field[MAX_LINE_LENGTH + 1];
    /** Length of last read field. */
    int field_len;
    /** Is current line labeled? */
    int labeled;
    /** Label of current line. */
    char label[MAX_LABEL_LENGTH + 1];
    /** Length of label of current line. */
    int label_len;
    /** Head of linked list of data symbols. */
    datasym_t *data_symbols;
} state_t;

/**
 * Parsed operand.
 */
typedef struct {
    /** Addressing mode. */
    addr_mode_t addr_mode;
    /** Referenced label. */
    char label[MAX_LABEL_LENGTH + 1];
    /** Value. */
    union {
        /** Immediate value. */
        word_t immediate;
        /** Referenced register. */
        word_t reg;
    } value;
} operand_t;

/**
 * Return values for parse_operand.
 */
typedef enum {
    PARSE_OPERAND_OK,   /**< Operand parsed successfully. */
    PARSE_OPERAND_BAD,  /**< Token was not a valid operand. */
    PARSE_OPERAND_EMPTY /**< Token was blank. */
} parse_operand_result_t;

/**
 * Prints a nicely formatted error with line number.
 */
static void print_error(state_t *st, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("firstpass: error: line %d: ", st->line_no);
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

/**
 * Processes the first field of a labeled line.
 *
 * @param st Internal state.
 * @param shared Shared state.
 */
static int process_label_field(state_t *st, shared_t *shared)
{
    int bad = 0; /* Label badly formatted? */
    const char *head = st->field;

    /* Reset length to 0. */
    st->label_len = 0;

    while ((st->label[st->label_len++] = *head++) != ':') {
        /* Check if symbol too long. */
        if (st->label_len > MAX_LABEL_LENGTH) {
            print_error(st, "label is too long (%d chars, max. %d).", st->label_len, MAX_LABEL_LENGTH);
            bad = 1; /* Flag symbol as bad. */
            break;
        }

        /* Check if alphanumeric. */
        if (!isalnum(st->label[st->label_len - 1])) {
            print_error(st, "invalid character '%c' in label (only alphanumeric characters allowed)",
                st->label[st->label_len - 1]);
            bad = 1; /* Flag symbol as bad. */
            break;
        }
    }

    if (bad)
        return 1;

    /* Check if symbol empty. */
    if (st->label_len <= 0) {
        print_error(st, "label is empty.");
        return 1;
    }

    /* Overwrite ':' with a null terminator. */
    st->label[st->label_len - 1] = '\0';

    if (symtable_find(shared->symtable, st->label)) {
        print_error(st, "label %s already defined; ignoring statement.", st->label);
        return 1;
    }

    return 0;
}

/**
 * Reads the next field and sets the eol flag if end of line reached.
 *
 * @param st Internal state.
 * @return Non-zero if end of line, else zero.
 */
static int get_next_field(state_t *st)
{
    read_field(&st->line_head, st->field, &st->field_len);
    return is_eol(*st->line_head);
}

/**
 * Inserts a symbol at the head of a linked list of data symbols.
 *
 * @param head Pointer to head node in list. Will be modified.
 * @param sym Symbol to insert.
 */
static void insert_data_symbol(datasym_t **head, symbol_t *sym)
{
    /* Allocate data symbol. */
    datasym_t *node = malloc(sizeof(datasym_t));

    /* Set symbol. */
    node->sym = sym;

    /* Make next pointer point to old head and replace head with new node. */
    node->next = *head;
    *head = node;
}

/**
 * Frees a linked list of data symbols.
 *
 * @param head Head node.
 */
static void free_data_symbols(datasym_t *head)
{
    datasym_t *cur, *next;

    for (cur = head; cur; cur = next) {
        next = cur->next;
        free(cur);
    }
}

/**
 * Recalculate addresses of data symbols using the code segment length as an
 * offset. This is needed because the data segment appears directly after the
 * code segment in the object file.
 *
 * @param head Head of linked list to update.
 * @param offset Word offset at which data segment starts in object file.
 */
static void update_data_symbols(datasym_t *head, word_t offset)
{
    datasym_t *cur; /* Current node. */
    word_t new_address; /* Recalculated address of symbol. */
    
    for (cur = head; cur; cur = cur->next) {
        /* Calculate new address. */
        new_address = offset + cur->sym->base_addr + cur->sym->offset;

        /* Set new base address and offset. */
        cur->sym->base_addr = SYMBOL_BASE_ADDR(new_address);
        cur->sym->offset = SYMBOL_OFFSET(new_address);
    }
}

/**
 * Process data after a .data directive.
 *
 * @param st Internal state.
 * @param shared Shared state.
 */
static int process_data_directive(state_t *st, shared_t *shared)
{
    char c; /* Current character. */
    int len; /* Number of words read. */
    symbol_t *sym; /* Symbol. */

    /* Skip whitespace. */
    while ((c = *st->line_head++) != '\0' && isspace(c))
        ;

    /* Unread last character. */
    --st->line_head;

    /* If last character was end of line then no data is present. */
    if (is_eol(c)) {
        print_error(st, "missing data after data directive.");
        return 1;
    }
    
    /* Read comma separated integer values into data segment. */
    len = read_comma_separated_data(
        st->line_head,
        shared->data_seg + st->dc,
        MAX_DATA_LENGTH);

    /* Check if bad data. */
    if (len == -1) {
        print_error(st, "invalid data after data directive.");
        return 1;
    }

    /* Check if empty data. */
    if (len == 0) {
        print_error(st, "no data after data directive.");
        return 1;
    }

    /* Add symbol if labeled. */
    if (st->labeled) {
        sym = symtable_new(shared->symtable, st->label);
        assert(sym);
        sym->data = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(st->dc);
        sym->offset = SYMBOL_OFFSET(st->dc);

        /* Insert to linked list of data symbols. */
        insert_data_symbol(&st->data_symbols, sym);
    }

    /* Increment data counter. */
    st->dc += len;

    return 0;
}

/**
 * Process data after a .string directive.
 *
 * @param st Internal state.
 * @param shared Shared state.
 */
static int process_string_directive(state_t *st, shared_t *shared)
{
    char c; /* Last read string directive character. */
    const int addr = st->dc; /* Address of string. */
    symbol_t *sym; /* Symbol. */

    /* Skip whitespace. */
    while ((c = *st->line_head++) != '\0' && isspace(c))
        ;

    /* Unread last character. */
    --st->line_head;

    /* If last character was end of line then no data is present. */
    if (is_eol(c)) {
        print_error(st, "missing string data after string directive.");
        return 1;
    }

    /* String needs to begin with double quotes. */
    if (*st->line_head++ != '"') {
        print_error(st, "string data missing opening double quotes.");
        return 1;
    }

    /* Copy string into data segment, incrementing the data counter for
       every character (word) written. */
    while ((c = *st->line_head++) != '\0' && c != '"')
        shared->data_seg[st->dc++] = c;

    /* Check if string is improperly terminated. */
    if (c != '"') {
        print_error(st, "string data missing closing quotes.");
        return 1;
    }

    /* Append null terminator. */
    shared->data_seg[st->dc++] = '\0';

    /* Add symbol if labeled. */
    if (st->labeled) {
        sym = symtable_new(shared->symtable, st->label);
        assert(sym);
        sym->data = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(addr);
        sym->offset = SYMBOL_OFFSET(addr);

        /* Insert to linked list of data symbols. */
        insert_data_symbol(&st->data_symbols, sym);
    }

    return 0;
}

static parse_operand_result_t parse_operand(state_t *st, const char *tok, operand_t *op)
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
        /* Apply addressing mode. */
        op->addr_mode = ADDR_MODE_IMMEDIATE;

        /* Parse immediate value. */
        if (parse_number(tok + 1, &op->value.immediate) != 0) {
            print_error(st, "could not parse immediate number in operand.");
            return PARSE_OPERAND_BAD;
        }

        return PARSE_OPERAND_OK;
    }

    /* Check if direct register mode. */
    if (tok[0] == 'r' && parse_number(tok + 1, &op->value.reg) == 0) {
        /* Apply addressing mode. */
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
            print_error(st, "label too long.");
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
            print_error(st, "label is empty.");
            return PARSE_OPERAND_BAD;
        }

        /* Apply addressing mode. */
        op->addr_mode = ADDR_MODE_DIRECT;

        /* Append null terminator. */
        op->label[label_len] = 0;

        return PARSE_OPERAND_OK;
    }

    if (!isspace(c) && c != '[') {
        /* Found non-alphanumeric character in label. */
        print_error(st, "invalid label (non-alphanumeric character: \'%c\').\n", c);
        return PARSE_OPERAND_BAD;
    }

    /* Check for extraneous characters. */
    while (!is_eol(c = *tok++) && isspace(c))
        ;
    if (!is_eol(c) && c != '[') {
        /* Found extraneous character. */
        print_error(st, "direct addressing operand has extraneous characters.");
        return PARSE_OPERAND_BAD;
    }

    /* Unread last character. */
    --tok;

    /* Null terminate label. */
    op->label[label_len] = '\0';

    /* Check if we have a register subscript. */
    if (c == '[') {
        /* Read register index. */
        if (sscanf(tok, "[r%d]", &reg_id) != 1) {
            print_error(st, "could not read register value from brackets.");
            return PARSE_OPERAND_BAD;
        }

        /* Check if offset register is valid. */
        if (reg_id < 0 || reg_id > 15) {
            print_error(st, "register value out of range: %d (must be between 0 and 15)\n", (int)op->value.reg);
            return PARSE_OPERAND_BAD;
        }

        /* Store register ID in operand. */
        op->value.reg = (word_t)reg_id;

        /* Apply addressing mode. */
        op->addr_mode = ADDR_MODE_INDEX;

        return PARSE_OPERAND_OK;
    }

    /* Apply addressing mode. */
    op->addr_mode = ADDR_MODE_DIRECT;

    return PARSE_OPERAND_OK;
}

/**
 * Process operands beginning at the head pointer.
 *
 * @param st Internal state.
 * @param ops Output array of operands.
 * @return Number of operands read or -1 on failure.
 * @note This destroys the line buffer so the line should not be
 *       accessed afterward.
 */
static int process_operands(state_t *st, operand_t ops[])
{
    char *tok; /* Token. */
    int nops = 0; /* Number of operands. */
    int parse_result; /* Result returned from parse_operand. */

    /* First token. */
    tok = strtok(st->line_head, ",");

    while (tok) {
        /* Check if too many operands. */
        if (nops >= MAX_OPERANDS) {
            print_error(st, "too many operands.");
            return -1;
        }

        /* Parse operand from token. */
        parse_result = parse_operand(st, tok, &ops[nops]);
        
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

/**
 * Converts an addressing mode to its machine code representation.
 *
 * @param addr_mode Internal addressing mode.
 */
static int addr_mode_to_index(addr_mode_t addr_mode)
{
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

/**
 * Writes or reserves extra words for an operand in a machine instruction.
 * Some of the words are completed later in the second pass.
 *
 * @param st Internal state.
 * @param shared Shared state.
 * @param op Operand for which to write/reserve extra words.
 */
static void write_extra_words(state_t *st, shared_t *shared, const operand_t *op)
{
    switch (op->addr_mode) {
    case ADDR_MODE_IMMEDIATE: 
        /* Write extra word containing the immediate value with A flag set. */
        shared->code_seg[st->ic++] = MAKE_EXTRA_INST_WORD(
            op->value.immediate, /* Value */
            0, /* E flag */
            0, /* R flag */
            1  /* A flag */
        );
        break;
    case ADDR_MODE_DIRECT:
        /* Preserve space for two extra words, filled later in second pass. */
        st->ic += 2;
        break;
    case ADDR_MODE_INDEX:
        /* Preserve space for two extra words, filled later in second pass. */
        st->ic += 2;
        break;
    case ADDR_MODE_REGISTER_DIRECT:
        /* No extra words needed, register stored in second word. */
        break;
    }
}

/**
 * Process an instruction statement.
 *
 * @param st Internal state.
 * @param shared Shared state.
 */
static int process_instruction(state_t *st, shared_t *shared)
{
    const inst_desc_t *desc; /* Instruction code. */
    operand_t ops[MAX_OPERANDS]; /* Operands. */
    int nops; /* Number of operands. */
    symbol_t *sym; /* Symbol. */
    inst_data_t *data; /* Pointer to data object in shared state for use by second pass. */
    int i; /* Counter. */
    int src_reg, dst_reg; /* Source and destination register numbers for encoding second instruction. */
    
    /* Look up instruction name by mnemonic. */
    desc = find_inst(st->field);

    /* No such mnemonic. */
    if (!desc) {
        print_error(st, "bad instruction mnemonic: %s", st->field);
        return 1;
    }

    /* Parse operands. */
    if ((nops = process_operands(st, ops)) < 0)
        return 1;

    if (desc->noperands != nops) {
        print_error(st, "incorrect number of operands (expected %d, got %d)", desc->noperands, nops);
        return 1;
    }

    /* Get data pointer and increment instruction count. */
    data = &shared->instructions[shared->instruction_count++];
    
    /* Store number of operands. */
    data->num_operands = desc->noperands;

    /* Store instruction address before incrementing IC. */
    data->ic = st->ic;

    /* Write first word (opcode). */
    shared->code_seg[st->ic++] = MAKE_FIRST_INST_WORD(
        INST_OPCODE(desc->instruction), /* Opcode */
        0, /* E flag */
        0, /* R flag */
        1  /* A flag */
    );

    if (nops > 0) {
        for (i = 0; i < nops; ++i) {
            /* Check if the addressing mode used is legal by checking if its bit
               is set in the addressing modes bitfield of the description. */
            if (!(desc->addr_modes[i] & ops[i].addr_mode)) {
                print_error(st, "operand %d has invalid addressing mode.", i + 1);
                return 1;
            }
    
            if (ops[i].addr_mode & (ADDR_MODE_DIRECT | ADDR_MODE_INDEX)) {
                /* Store referenced label. */
                strcpy(data->operand_symbols[i], ops[i].label);
            } else {
                /* Other modes don't reference a label so set to empty
                   string. */
                data->operand_symbols[i][0] = 0;
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
        shared->code_seg[st->ic++] = MAKE_SECOND_INST_WORD(
            nops == 1 ? addr_mode_to_index(ops[0].addr_mode) : addr_mode_to_index(ops[1].addr_mode), /* dst addr mode */
            dst_reg, /* dst register */
            nops == 2 ? addr_mode_to_index(ops[0].addr_mode) : 0, /* src addr mode */
            src_reg, /* src register */
            INST_FUNCT(desc->instruction), /* funct */
            0, /* E flag */
            0, /* R flag */
            1  /* A flag */
        );

        /* Write/preserve extra words for first operand. */
        write_extra_words(st, shared, &ops[0]);

        /* Write/preserve extra words for second operand if specified. */
        if (nops > 1)
            write_extra_words(st, shared, &ops[1]);
    }

    if (st->labeled) {
        /* Make a symbol for the instruction. */
        sym = symtable_new(shared->symtable, st->label);
        assert(sym);
        sym->code = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(data->ic);
        sym->offset = SYMBOL_OFFSET(data->ic);
    }

    return 0;
}

/**
 * Process a line of expanded assembly code.
 *
 * @param st Internal state.
 * @param shared Shared state.
 * @param line Line to process.
 * @return Zero on success, non-zero on failure.
 */
static int process_line(state_t *st, shared_t *shared, char *line)
{
    symbol_t *sym;

    /* Increment line counter. */
    ++st->line_no;

    /* Reset line head to beginning of line. */
    st->line_head = line;

    /* First field. */
    if (get_next_field(st) != 0)
        return 0; /* Skip empty line. */

    /* Handle comment lines. */
    if (st->field[0] == ';')
        return 0; /* Skip comment line. */

    /* Check if first field is a label. */
    if (st->field[st->field_len - 1] == ':') {
        /* Try to read as label. */
        if (process_label_field(st, shared) != 0)
            return 1;

        /* Labeled. */
        st->labeled = 1;

        /* Next field. */
        if (get_next_field(st) != 0)
            return 0; /* Line has no directives/instructions. */
    } else {
        /* Not labeled. */
        st->labeled = 0;
    }

    if (st->field[0] == '.') {
        /* Process directives. */
        if (strcmp(st->field + 1, "data") == 0) {
            /* Data directive. */
            if (process_data_directive(st, shared))
                return 1;
        } else if (strcmp(st->field + 1, "string") == 0) {
            /* String directive. */
            if (process_string_directive(st, shared))
                return 1;
        } else if (strcmp(st->field + 1, "extern") == 0) {
            /* Read label. */
            get_next_field(st); /* We don't care about end of line. */

            /* Check if label is missing. */
            if (st->field[0] == '\0') {
                print_error(st, ".extern directive missing label reference.");
                return 1;
            }

            /* Insert a symbol with external flag  and address and offset
            set to zero. */
            sym = symtable_new(shared->symtable, st->field);
            assert(sym);
            sym->ext = 1;
            sym->base_addr = 0;
            sym->offset = 0;
        } else if (strcmp(st->field + 1, "entry") == 0) {
            /* Entry directives ignored in first pass. */
        } 
    } else if (!is_eol(st->field[0])) {
        /* Not a directive yet field is not empty so must be a
           instruction. */
        if (process_instruction(st, shared) != 0)
            return 1;
    } else {
        /* End of line after first field, must be empty label. */
        assert(st->labeled);

        /* Try to allocate a symbol. */
        sym = symtable_new(shared->symtable, st->label);
        assert(sym);
        sym->code = 1;
        sym->base_addr = SYMBOL_BASE_ADDR(st->ic);
        sym->offset = SYMBOL_OFFSET(st->ic);
    }

    return 0;
}

int firstpass(const char *filename, struct shared *shared)
{
    FILE *in; /* Input file pointer. */
    char line[MAX_LINE_LENGTH + 1]; /* Line buffer. */
    state_t st; /* Internal state. */
    int error = 0; /* Error flag. */

    /* Try to open input file. */
    if ((in = fopen(filename, "r")) == 0) {
        printf("error: firstpass: could not open input file %s.\n", filename);
        return 1;
    }

    /* Zero initialize internal state. */
    memset(&st, 0, sizeof(st));

    /* Code segment is loaded at 100 so initialize IC to 100. */
    st.ic = 100;

    /* Process file line by line. */
    while (fgets(line, sizeof(line), in))
        error |= process_line(&st, shared, line);

    /* Close input file. */
    fclose(in);

    /* Save segment lengths. */
    shared->code_seg_len = st.ic;
    shared->data_seg_len = st.dc;

    /* Update data symbol addresses and free the list of data symbols. */
    update_data_symbols(st.data_symbols, st.ic);
    free_data_symbols(st.data_symbols);

    return error;
}
