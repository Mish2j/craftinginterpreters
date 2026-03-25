//> Chunks of Bytecode chunk-c
#include <stdlib.h>

#include "chunk.h"
//> chunk-c-include-memory
#include "memory.h"
//< chunk-c-include-memory
//> Garbage Collection chunk-include-vm
#include "vm.h"
//< Garbage Collection chunk-include-vm

#include <stdio.h>
#include <stdlib.h>

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
//> chunk-null-lines
  chunk->linesCount = 0;
  chunk->linesCapacity = 0;
  chunk->lines = NULL;
//< chunk-null-lines
//> chunk-init-constant-array
  initValueArray(&chunk->constants);
//< chunk-init-constant-array
}
//> free-chunk
void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
//> chunk-free-lines
  FREE_ARRAY(LineRun, chunk->lines, chunk->linesCapacity);
//< chunk-free-lines
//> chunk-free-constants
  freeValueArray(&chunk->constants);
//< chunk-free-constants
  initChunk(chunk);
}
//< free-chunk
/* Chunks of Bytecode write-chunk < Chunks of Bytecode write-chunk-with-line
void writeChunk(Chunk* chunk, uint8_t byte) {
*/
//> write-chunk
//> write-chunk-with-line
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
//< write-chunk-with-line
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code,
        oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->count++;

  if (chunk->linesCount > 0 &&
      chunk->lines[chunk->linesCount - 1].line == line) {
    chunk->lines[chunk->linesCount - 1].count++;
  } else {
    if (chunk->linesCapacity < chunk->linesCount + 1) {
      int oldCapacity = chunk->linesCapacity;
      chunk->linesCapacity = GROW_CAPACITY(oldCapacity);
      chunk->lines = GROW_ARRAY(LineRun, chunk->lines,
                                oldCapacity, chunk->linesCapacity);
    }

    chunk->lines[chunk->linesCount].line = line;
    chunk->lines[chunk->linesCount].count = 1;
    chunk->linesCount++;
  }
}
//< write-chunk
void writeConstant(Chunk* chunk, Value value, int line) {
  int constant = addConstant(chunk, value);

  if (constant <= 0xff) {
    writeChunk(chunk, OP_CONSTANT, line);
    writeChunk(chunk, (uint8_t)constant, line);
  } else if (constant <= 0xffffff) {
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    writeChunk(chunk, (constant >> 16) & 0xff, line);
    writeChunk(chunk, (constant >> 8) & 0xff, line);
    writeChunk(chunk, constant & 0xff, line);
  } else {
    fprintf(stderr, "Too many constants in one chunk.\n");
    exit(1);
  }
}
//> add-constant
int addConstant(Chunk* chunk, Value value) {
//> Garbage Collection add-constant-push
  push(value);
//< Garbage Collection add-constant-push
  writeValueArray(&chunk->constants, value);
//> Garbage Collection add-constant-pop
  pop();
//< Garbage Collection add-constant-pop
  return chunk->constants.count - 1;
}
//< add-constant

int getLine(Chunk* chunk, int instruction) {
  int current = 0;

  for (int i = 0; i < chunk->linesCount; i++) {
    current += chunk->lines[i].count;
    if (instruction < current) {
      return chunk->lines[i].line;
    }
  }

  return -1;
}