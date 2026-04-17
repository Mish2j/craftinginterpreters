//> Chunks of Bytecode main-c
//> Scanning on Demand main-includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//< Scanning on Demand main-includes
#include "common.h"
//> main-include-chunk
#include "chunk.h"
//< main-include-chunk
//> main-include-debug
#include "debug.h"
//< main-include-debug
//> A Virtual Machine main-include-vm
#include "vm.h"
//< A Virtual Machine main-include-vm
//> Scanning on Demand repl

#include <time.h>
#include "table.h"
#include "object.h"
#include "value.h"


static double nowMs(void) {
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
}

static Value makeKey(int i) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "k%d", i);
    return OBJ_VAL(copyString(buffer, len));
}

static void benchmarkBasic(int n) {
    Table table;
    initTable(&table);

    double start = nowMs();
    for (int i = 0; i < n; i++) {
        tableSet(&table, makeKey(i), NUMBER_VAL(i));
    }
    double mid = nowMs();

    Value v;
    for (int i = 0; i < n; i++) {
        tableGet(&table, makeKey(i), &v);
    }
    double end = nowMs();

    printf("Basic (%d)\n", n);
    printf("  Insert: %.2f ms\n", mid - start);
    printf("  Lookup: %.2f ms\n", end - mid);

    freeTable(&table);
}

static void benchmarkRandom(int n) {
    Table table;
    initTable(&table);

    for (int i = 0; i < n; i++) {
        tableSet(&table, makeKey(i), NUMBER_VAL(i));
    }

    Value v;
    double start = nowMs();
    for (int i = 0; i < n; i++) {
        int r = rand() % n;
        tableGet(&table, makeKey(r), &v);
    }
    double end = nowMs();

    printf("Random lookup (%d)\n", n);
    printf("  Lookup: %.2f ms\n", end - start);

    freeTable(&table);
}

static void benchmarkDelete(int n) {
    Table table;
    initTable(&table);

    for (int i = 0; i < n; i++) {
        tableSet(&table, NUMBER_VAL(i), NUMBER_VAL(i));
    }

    double start = nowMs();
    for (int i = 0; i < n; i += 2) {
        tableDelete(&table, NUMBER_VAL(i));
    }
    double mid = nowMs();

    for (int i = 0; i < n; i += 2) {
        tableSet(&table, NUMBER_VAL(i), NUMBER_VAL(i));
    }
    double end = nowMs();

    printf("Delete-heavy (%d)\n", n);
    printf("  Delete: %.2f ms\n", mid - start);
    printf("  Reinsert: %.2f ms\n", end - mid);

    freeTable(&table);
}

static void runBenchmarks(void) {
    printf("Running hash table benchmarks...\n\n");

    benchmarkBasic(10000);
    printf("\n");

    benchmarkRandom(10000);
    printf("\n");

    benchmarkDelete(10000);
    printf("\n");
}

static void repl() {
  char line[1024];
  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line);
  }
}
//< Scanning on Demand repl
//> Scanning on Demand read-file
static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
//> no-file
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }
//< no-file

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);
//> no-buffer
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }
  
//< no-buffer
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
//> no-read
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  
//< no-read
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}
//< Scanning on Demand read-file
//> Scanning on Demand run-file
static void runFile(const char* path) {
  char* source = readFile(path);
  InterpretResult result = interpret(source);
  free(source); // [owner]

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}
//< Scanning on Demand run-file

int main(int argc, const char* argv[]) {
//> A Virtual Machine main-init-vm
  initVM();

//< A Virtual Machine main-init-vm
/* Chunks of Bytecode main-chunk < Scanning on Demand args
  Chunk chunk;
  initChunk(&chunk);
*/
/* Chunks of Bytecode main-constant < Scanning on Demand args

  int constant = addConstant(&chunk, 1.2);
*/
/* Chunks of Bytecode main-constant < Chunks of Bytecode main-chunk-line
  writeChunk(&chunk, OP_CONSTANT);
  writeChunk(&chunk, constant);

*/
/* Chunks of Bytecode main-chunk-line < Scanning on Demand args
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
*/
/* A Virtual Machine main-chunk < Scanning on Demand args

  constant = addConstant(&chunk, 3.4);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_ADD, 123);

  constant = addConstant(&chunk, 5.6);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_DIVIDE, 123);
*/
/* A Virtual Machine main-negate < Scanning on Demand args
  writeChunk(&chunk, OP_NEGATE, 123);
*/
/* Chunks of Bytecode main-chunk < Chunks of Bytecode main-chunk-line
  writeChunk(&chunk, OP_RETURN);
*/
/* Chunks of Bytecode main-chunk-line < Scanning on Demand args

  writeChunk(&chunk, OP_RETURN, 123);
*/
/* Chunks of Bytecode main-disassemble-chunk < Scanning on Demand args

  disassembleChunk(&chunk, "test chunk");
*/
/* A Virtual Machine main-interpret < Scanning on Demand args
  interpret(&chunk);
*/
//> Scanning on Demand args
    if (argc == 2 && strcmp(argv[1], "--bench") == 0) {
        runBenchmarks();
    }
    else if (argc == 1) {
     repl();
    }
    else if (argc == 2) {
        runFile(argv[1]);
    }
    else {
        fprintf(stderr, "Usage: clox [path] or clox --bench\n");
        exit(64);
    }
  
  freeVM();
//< Scanning on Demand args
/* A Virtual Machine main-free-vm < Scanning on Demand args
  freeVM();
*/
/* Chunks of Bytecode main-chunk < Scanning on Demand args
  freeChunk(&chunk);
*/
  return 0;
}
