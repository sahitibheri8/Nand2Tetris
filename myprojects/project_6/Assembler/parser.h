#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>

/* One label entry: key → value */
typedef struct {
    char *key;
    char *value;
} Label;

/* Parser object */
typedef struct {
    FILE    *file;
    char   **commands;
    int      count, capacity;
    int      index, counter, next_var;
    Label   *labels;
    int      label_count, label_cap;
} Parser;

/* Public API */
Parser  *parser_create(const char *asm_path);
bool     parser_has_more(Parser *p);
char    *parser_advance(Parser *p);
char    *parser_current(Parser *p);
char    *parser_label_lookup(Parser *p, const char *instr);
void     parser_destroy(Parser *p);

#endif /* PARSER_H */
