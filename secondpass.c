/**
 * @file secondpass.h
 * @author Tamir Attias
 * @brief Second pass definitions.
 */

#include "secondpass.h"
#include "constants.h"
#include "shared.h"
#include "ioutil.h"
#include "symtable.h"
#include "instset.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
 * Internal state for second pass.
 */
typedef struct {
    /** Line number. */
    int line_no;
    /** Pointer to next character to process. */
    char *line_head;
    /** Last read field. */
    char field[MAX_LINE_LENGTH + 1];
    /** Length of last read field. */
    int field_len;
    /** Index of next instruction to process. */
    int instruction_index; 
} state_t;

/**
 * Prints a nicely formatted error with line number.
 */
static void print_error(state_t *st, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("secondpass: error: line %d: ", st->line_no);
    vprintf(fmt, args);
    putchar('\n'); /* Newline at end. */
    va_end(args);
}

/**
 * Reads the next field and sets the eol flag if end of line reached.
 *
 * @return Non-zero if end of line, else zero.
 */
static int get_next_field(state_t *st)
{
    read_field(&st->line_head, st->field, &st->field_len);
    return is_eol(*st->line_head);
}

/**
 * Adds missing words for an instruction in the code segment, if necessary.
 *
 * @param st Internal state.
 * @param shared Data shared between first and second pass.
 * @param data Instruction data.
 * @return Zero on success, non-zero on failure.
 */
static int complete_instruction(state_t *st, struct shared *shared, const inst_data_t *data)
{
    int i; /* Counter. */
    symbol_t *sym; /* Referenced symbol. */
    word_t *words = &shared->code_seg[data->ic]; /* Pointer to first word of instruction. */

    for (i = 0; i < data->num_operands; ++i) {
        /* If referenced label is empty then the operand uses direct
           addressing. */
        if (data->operand_symbols[i][0] == '\0')
            continue;
        
        /* Find symbol referenced by operand. */
        sym = symtable_find(shared->symtable, data->operand_symbols[i]);
        if (!sym) {
            print_error(st, "could not find symbol %s referenced by operand #%d.",
                data->operand_symbols[i], i + 1);
            return 1;
        }

        /* Third word is base address. */
        words[2] = MAKE_EXTRA_INST_WORD(
            sym->base_addr,   /* Value */
            sym->ext ? 1 : 0, /* E flag */
            1,                /* R flag */
            0                 /* A flag */
        );

        /* Fourth word is offset from base address. */
        words[3] = MAKE_EXTRA_INST_WORD(
            sym->offset,      /* Value */
            sym->ext ? 1 : 0, /* E flag */
            1,                /* R flag */
            0                 /* A flag */
        );

        /* TODO: If external, add to list of external words. */
    }

    return 0;
}

/**
 * Process a line of expanded assembly code.
 *
 * @param st Internal state.
 * @param shared Shared state.
 * @param line Line to process.
 */
static int process_line(state_t *st, shared_t *shared, char *line)
{
    symbol_t *sym; /* Symbol referenced by .entry directive. */

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

    /* Check if labeled. */
    if (st->field[st->field_len - 1] == ':') {
        /* Skip the label without checking for validity as that was already
           done in the first pass. */
        if (get_next_field(st) != 0)
            return 0; /* Nothing after label, ignore line. */
    }

    /* Check if directive. */
    if (st->field[0] == '.') {
        /* Check if .entry directive. */
        if (strcmp(st->field + 1, "entry") != 0)
            return 0; /* Skip non .entry directives. */

        /* Next field is the symbol name. */
        if (get_next_field(st) != 0) {
            print_error(st, "missing symbol name in .entry directive.");
            return 1;
        }

        /* Try to find the symbol in the symbol table. */
        sym = symtable_find(shared->symtable, st->field);
        if (!sym) {
            print_error(st, "could not find symbol %s in symbol table.", st->field);
            return 1;
        }
        
        /* Mark as entry. */
        sym->ent = 1;
    } else {
        /* Instruction statement. Fill in missing words. */
        if (complete_instruction(st, shared, &shared->instructions[st->instruction_index++]) != 0)
            return 1;
    }

    return 0;
}

static int write_segment(FILE *fp, const word_t *segment, int base_addr, int len)
{
    int i;
    word_t w;

    for (i = 0; i < len; ++i) {
        /* Get word. */
        w = segment[i];

        /* Write address. */
        fprintf(fp, "%04d ", base_addr + i);

        /* Encode it to file. */
        fprintf(fp, "A%x-B%x-C%x-D%x-E%x\n",
            (unsigned)((w >> 16) & 0xF),
            (unsigned)((w >> 12) & 0xF),
            (unsigned)((w >> 8)  & 0xF),
            (unsigned)((w >> 4)  & 0xF),
            (unsigned)((w >> 0)  & 0xF)
        );
    }

    return 0;
}

static int write_object_file(const char *obfilename, struct shared *shared)
{
    FILE *out; /* Output file pointer. */
    int error = 0; /* Return value. */

    /* Try to open file for writing. */
    if ((out = fopen(obfilename, "wb")) == 0) {
        printf("secondpass: could not open object file %s for writing\n", obfilename);
        return 1;
    }

    /* Write header. */
    if (fprintf(out, "%d %d\n", shared->code_seg_len, shared->data_seg_len) < 0) {
        printf("secondpass: error: could not write header.\n");
        goto done;
    }

    /* Write code segment. */
    if ((error = write_segment(out, shared->code_seg, 100, shared->code_seg_len)) != 0) {
        printf("secondpass: error: could not write code segment.\n");
        goto done;
    }

    /* Write data segment. */
    if ((error = write_segment(out, shared->data_seg, 100 + shared->code_seg_len, shared->data_seg_len)) != 0) {
        printf("secondpass: error: could not write data segment.\n");
        goto done;
    }

done:
    /* Close output file. */
    fclose(out);

    return error;
}

int secondpass(const char *infilename, const char *obfilename, struct shared *shared)
{
    FILE *in; /* Input file pointer. */
    char line[MAX_LINE_LENGTH + 1]; /* Line buffer. */
    state_t st; /* Internal state. */
    int error = 0; /* Return value. */

    /* Try to open input file. */
    if ((in = fopen(infilename, "r")) == 0) {
        printf("secondpass: error: could not open %s\n", infilename);
        return 1;
    }

    /* Initialize state to zero. */
    memset(&st, 0, sizeof(st));

    /* Process file line by line. */
    while (fgets(line, sizeof(line), in))
        error |= process_line(&st, shared, line);

    /* Close input file. */
    fclose(in);

    /* Terminate early if an error was encountered. */
    if (error)
        return error;

    /* Write machine code to object file. */
    if ((error = write_object_file(obfilename, shared)) != 0)
        return error;

    return 0;
}

