#ifndef WRITER_H
#define WRITER_H

#include "parser.h"
#include <stdio.h>

typedef struct {
    Parser *parser;
    FILE   *out_fp;
} Writer;

/* Create a writer that parses `asm_path` and writes to `hack_path` */
Writer *writer_create(const char *asm_path, const char *hack_path);

/* Emit all parsed instructions as 16-bit binary lines */
void    writer_write(Writer *w);

/* Clean up resources */
void    writer_destroy(Writer *w);

#endif // WRITER_H
