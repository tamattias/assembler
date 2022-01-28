/**
 * @file firstpass.c
 * @author Tamir Attias
 * @brief First pass implementation.
 */

#include "firstpass.h"
#include "constants.h"
#include "ioutil.h"
#include "shared.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/**
 * First pass internal state.
 */
typedef struct {
    int ic; /** Instruction counter. */
    int dc; /** Data counter. */
    int line_no; /** Current line number. */
    char *line_head; /** Line head. */
    int eol; /** End of line reached? */
    char field[MAX_LINE_LENGTH + 1]; /* Current field. */
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
 * @param data Pointer to an array of integers in which to store the read
 *             data.
 * @param limit Maximum number of items that can be read into data.
 * @note Only the first MAX_LINE_LENGTH bytes of data will be scanned.
 * @return Number of integers read or -1 if invalid data.
 */
static int read_comma_separated_data(const char *input, int *data, int limit)
{
    char tmpstr[MAX_LINE_LENGTH + 1]; /* Copy of input for tokenization. */
    char *tok; /* Current token. */
    int nval; /* Integer parsed from current token. */
    int count = 0; /* Tokens read successfully so far. */
    
    /* Make a copy of the input for tokenization. */
    strncpy(tmpstr, input, MAX_LINE_LENGTH);

    /* Begin tokenization. */
    tok = strtok(tmpstr, ",");
    while (tok) {
        /* Read integer from token. */
        if (sscanf(tok, "%d", &nval) != 1)
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

static int process_label_field(firstpass_t *fp)
{
    int bad; /* Label badly formatted? */
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
            report_error(fp, "invalid character in label (only alphanumeric characters allowed)");
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

    /* TODO: Check if symbol already defined in symbol table. */

    return 0;
}

/**
 * Reads the next field.
 *
 * @return Non-zero if end of line, else 0.
 */
static int next_field(firstpass_t *fp)
{
    return (fp->eol = read_field(&fp->line_head, fp->field, &fp->field_len));
}

/**
 * Process data after a .data directive.
 */
static void process_data_directive(firstpass_t *fp)
{
    int data[MAX_DATA_LENGTH];
    int data_len;

    if (fp->eol) {
        report_error(fp, "missing data after data directive.");
        return;
    }
    
    data_len = read_comma_separated_data(fp->line_head, data, MAX_DATA_LENGTH);

    if (data_len == 0) {
        report_error(fp, "no data after data directive.");
        return;
    }
}

/**
 * Process data after a .string directive.
 */
static void process_string_directive(firstpass_t *fp)
{
    char c; /* Last read string directive character. */
    char buf[MAX_LINE_LENGTH + 1]; /* String directive buffer. */
    int len; /* Number of characters in buf. */

    if (fp->eol) {
        report_error(fp, "missing string data after string directive.");
        return;
    }

    /* Skip whitespace. */
    while ((c = *fp->line_head++) != '\0' && isspace(c))
        ;

    /* If last character was end of line then no data is present. */
    if (is_eol(c)) {
        report_error(fp, "missing string data after string directive.");
        return;
    }

    /* Unread last character. */
    --fp->line_head;

    /* String needs to begin with double quotes. */
    if (*fp->line_head++ != '"') {
        report_error(fp, "string data missing opening double quotes.");
        return;
    }

    len = 0;

    /* Copy string into data buffer. */
    while ((c = *fp->line_head++) != '\0' && c != '"')
        buf[len++] = c;

    /* Check if string is improperly terminated. */
    if (c != '"') {
        report_error(fp, "string data missing closing quotes.");
        return;
    }

    /* Append null terminator. */
    buf[len] = '\0';
}

static void process_line(firstpass_t *fp, char *line, shared_t *shared)
{
    /* Increment line counter. */
    ++fp->line_no;

    /* Initialize head to beginning of line. */
    fp->line_head = line;

    /* First field. */
    if (next_field(fp) != 0)
        return; /* Skip empty line. */

    /* Handle comment lines. */
    if (fp->field[0] == ';')
        return; /* Skip comment line. */

    /* Check if first field is a label. */
    if (fp->field[fp->field_len] == ':') {
        /* Try to read as label. */
        if (process_label_field(fp) != 0)
            return;

        /* Labeled. */
        fp->labeled = 1;

        /* Next field. */
        if (next_field(fp) != 0)
            return;
    } else {
        /* Not labeled. */
        fp->labeled = 0;
    }

    if (strcmp(fp->field, ".data") == 0) {
        process_data_directive(fp);
    } else if (strcmp(fp->field, ".string") == 0) {
        process_string_directive(fp);
    }
}

int firstpass(const char *filename, struct shared *shared)
{
    FILE *in; /* Input file pointer. */
    char line[MAX_LINE_LENGTH + 1]; /* Line buffer. */
    firstpass_t fp;

    /* Try to open input file. */
    in = fopen(filename, "r");
    if (!in) {
        printf("error: firstpass: could not open input file %s.\n", filename);
        return 1;
    }

    /* Zero initialize first pass state. */
    memset(&fp, 0, sizeof(fp));

    /* Process file line by line. */
    while (fgets(line, sizeof(line), in))
        process_line(&fp, line, shared);

    /* Close input file. */
    fclose(in);

    return 0;
}
