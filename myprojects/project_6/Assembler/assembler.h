#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "writer.h"

/**
 * Encapsulates the input ASM path, output HACK path, and Writer instance.
 */
typedef struct {
    char   *asm_path;
    char   *hack_path;
    Writer *writer;
} Assembler;

/**
 * Create an Assembler for a given .asm file.
 * Returns NULL on extension error or allocation failure.
 */
Assembler *assembler_create(const char *asm_path);

/**
 * Run the translation: invoke the Writer to generate .hack.
 */
void assembler_compile(Assembler *a);

/**
 * Free all resources and close files.
 */
void assembler_destroy(Assembler *a);

#endif /* ASSEMBLER_H */
