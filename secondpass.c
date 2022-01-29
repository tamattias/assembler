/**
 * @file secondpass.h
 * @author Tamir Attias
 * @brief Second pass definitions.
 */

#include "secondpass.h"
#include "constants.h"
#include "shared.h"
#include "util.h"
#include "symtable.h"
#include "instset.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
 * Node in linked list of entry points.
 */
typedef struct entrypoint {
    /** Symbol name. */
    char label[MAX_LABEL_LENGTH + 1];
    /** Base address. */
    word_t base_addr;
    /** Offset from base address. */
    word_t offset;
    /** Next item in list of entry points. */
    struct entrypoint *next;
} entrypoint_t;

/**
 * Node in linked list of code words referencing external symbols.
 */
typedef struct external {
    /** Address of machine code word in which to load the base address of the
        symbol. */
    word_t base_addr_word_addr;
    /** Address of machine code word in which to load the offset from the base
        address of the symbol. */
    word_t offset_word_addr;
    /** Externally referenced symbol for this word. */
    char symbol[MAX_LABEL_LENGTH + 1];
    /** Next item in list of externals. */
    struct external *next;
} external_t;

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
    /** Head of entry point linked list. */
    entrypoint_t *entrypoints;
    /** Head of externals linked list. */
    external_t *externals;
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
 * Reads the next field in the line.
 *
 * @param st Internal state.
 */
static void next_field(state_t *st)
{
    read_field(&st->line_head, st->field, &st->field_len);
}

/**
 * Inserts an entry at the head of the externals list.
 *
 * @param head Pointer to head node. Will receive the new node.
 * @param base_addr_word_addr Address of machine code word in which the base
 *                            address of the symbol will be loaded.
 * @param offset_word_addr Address of machine code word in which the offset
 *                         from the base address of the symbol will be loaded.
 * @param symbol External symbol referenced by the machine code word.
 */
static void insert_external(
    external_t **head,
    word_t base_addr_word_addr, 
    word_t offset_word_addr,
    const char *symbol)
{
    /* Allocate node. */
    external_t *node = (external_t*)malloc(sizeof(external_t));
    
    /* Calculate base address and offset from word address. */
    node->base_addr_word_addr = base_addr_word_addr;
    node->offset_word_addr = offset_word_addr;

    /* Copy symbol name. */
    strcpy(node->symbol, symbol);

    /* Set next pointer to head. */
    node->next = *head;

    /* Replace head with new node. */
    *head = node;
}

/**
 * Free externals linked list.
 *
 * @param head Head of list of externals to free.
 */
static void free_externals(external_t *head)
{
    external_t *cur, *next;

    for (cur = head; cur; cur = next) {
        next = cur->next;
        free(cur);
    }
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
    word_t *words; /* Pointer to first word of instruction. */

    /* We subtract 100 to get the address relative to the beginning of the
       code segment. */
    words = &shared->code_seg[data->address - 100];

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
            sym->ext ? 0 : 1, /* R flag */
            0                 /* A flag */
        );

        /* Fourth word is offset from base address. */
        words[3] = MAKE_EXTRA_INST_WORD(
            sym->offset,      /* Value */
            sym->ext ? 1 : 0, /* E flag */
            sym->ext ? 0 : 1, /* R flag */
            0                 /* A flag */
        );

        if (sym->ext) {
            /* Store addresses of words where the symbol's base address and
               offset should be placed. */
            insert_external(
                &st->externals,
                data->address + 2,
                data->address + 3,
                data->operand_symbols[i]);
        }
    }

    return 0;
}

/**
 * Insert a new entrypoint at the head of a linked list of entrypoints.
 *
 * @param head Pointer to head of entry point list (will be modified.)
 * @param label Symbol name.
 * @param base_addr Base address.
 * @param offset Offset from base address.
 */
static void insert_entrypoint(entrypoint_t **head, const char *label, word_t base_addr, word_t offset)
{
    /* Allocate entry point. */
    entrypoint_t *ep = (entrypoint_t*)malloc(sizeof(entrypoint_t));
    
    /* Copy label. */
    strcpy(ep->label, label);

    /* Set address. */
    ep->base_addr = base_addr;
    ep->offset = offset;

    /* Set next to head. */
    ep->next = *head;

    /* Replace head with new entrypoint. */
    *head = ep;
}

/**
 * Free the linked list of entry points.
 *
 * @param head Head of linked list.
 */
static void free_entrypoints(entrypoint_t *head)
{
    entrypoint_t *cur, *next;

    /* Traverse list, freeing each node. */
    for (cur = head; cur; cur = next) {
        next = cur->next;
        free(cur);
    }
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
    next_field(st);

    /* Check if first field is empty. */
    if (st->field[0] == '\0')
        return 0; /* Skip empty line. */

    /* Handle comment lines. */
    if (st->field[0] == ';') {
        /* Check if line number reset generated by preprocessor. */
        if (st->field[1] == '#') {
            /* Reset line number to value after pound sign. */
            sscanf(st->field + 2, "%d", &st->line_no);
        }
        return 0; /* Skip comment line. */
    }

    /* Check if labeled. */
    if (st->field[st->field_len - 1] == ':') {
        /* Skip the label without checking for validity as that was already
           done in the first pass. */
        next_field(st);
        if (st->field[0] == '\0')
            return 0; /* Nothing after label, ignore line. */
    }

    /* Check if directive. */
    if (st->field[0] == '.') {
        /* Check if .entry directive. */
        if (strcmp(st->field + 1, "entry") != 0)
            return 0; /* Skip non .entry directives. */

        /* Get symbol name. */
        next_field(st);

        /* Check if symbol name is empty. */
        if (st->field[0] == '\0') {
            print_error(st, "missing symbol name in .entry directive.");
            return 1;
        }

        /* Try to find the symbol in the symbol table. */
        sym = symtable_find(shared->symtable, st->field);
        if (!sym) {
            print_error(st, "could not find symbol %s in symbol table.", st->field);
            return 1;
        }
        
        /* Insert to linked list of entry points. */
        insert_entrypoint(&st->entrypoints, st->field, sym->base_addr, sym->offset);
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

/**
 * Writes entry points to a .ent file.
 *
 * @param filename Output filename.
 * @param entrypoints Linked list of entry points to write.
 * @return Zero on success, non-zero on failure.
 */
static int write_entries_file(const char *filename, entrypoint_t *entrypoints)
{
    FILE *fp; /* Output file. */
    entrypoint_t *cur; /* Currently traversed entry. */

    /* Try to open the file. */
    if ((fp = fopen(filename, "w")) == 0) {
        printf("error: could not open entries file %s for writing\n", filename);
        return 1;
    }

    /* Traverse linked list of entrypoints. */
    for (cur = entrypoints; cur; cur = cur->next) {
        /* Write entry to file. */
        if (fprintf(fp, "%s,%ld,%ld\n", cur->label, (long)cur->base_addr, (long)cur->offset) < 0) {
            /* Report error. */
            printf("error: could not write entrypoint %s to entries file\n",
                cur->label);

            /* Close output file. */
            fclose(fp);

            return 1;
        }
    }

    /* Close output file. */
    fclose(fp);

    return 0;
}

/**
 * Writes externals to a .ext file.
 *
 * @param filename Output filename.
 * @param entrypoints Linked list of externals to write.
 * @return Zero on success, non-zero on failure.
 */
static int write_externals_file(const char *filename, external_t *externals)
{
    FILE *fp; /* Output file. */
    external_t *cur; /* Currently traversed entry. */
    int error = 0; /* Return value. */

    /* Try to open the file. */
    if ((fp = fopen(filename, "w")) == 0) {
        printf("error: could not open entries file %s for writing\n", filename);
        return 1;
    }

    /* Traverse linked list of externals. */
    for (cur = externals; cur; cur = cur->next) {
        /* Write base address and offset in separate lines. */
        if (fprintf(fp, "%s BASE %ld\n", cur->symbol, (long)cur->base_addr_word_addr) < 0 ||
            fprintf(fp, "%s OFFSET %ld\n", cur->symbol, (long)cur->offset_word_addr) < 0) {
            /* Report error. */
            printf("error: could not write external with symbol %s\n",
                cur->symbol);
    
            error = 1;
            goto end; /* Exit. */
        }

        /* Empty line between entries. */
        if (cur->next && fputc('\n', fp) == EOF) {
            /* Error in fputc. */

            error = 1;
            goto end; /* Exit. */
        }
    }

end:
    /* Close output file. */
    fclose(fp);

    return error;
}

int secondpass(
    const char *infilename,
    const char *obfilename,
    const char *entfilename,
    const char *extfilename,
    struct shared *shared
)
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

    if (error == 0 && st.entrypoints != 0) {
        /* Write entrypoints to .ent file. */
        write_entries_file(entfilename, st.entrypoints);

        /* Free linked list of entry points. */
        free_entrypoints(st.entrypoints);
    }

    if (error == 0 && st.externals != 0) {
        /* Write externals to .ext file. */
        write_externals_file(extfilename, st.externals);

        /* Free linked list of externals. */
        free_externals(st.externals);
    }

    /* Write machine code to object file. */
    if (error == 0)
        error = write_object_file(obfilename, shared);

    return error;
}

