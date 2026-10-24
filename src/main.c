#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define global_variable static
#define local_persist static
#define internal static

#define Assert(Expression)                                                     \
  if (!(Expression)) {                                                         \
    *(int *)0 = 0;                                                             \
  }

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef i8 b8;
typedef i32 b32;

typedef struct Arena Arena;
struct Arena {
  u8 *buffer;
  u64 len;
  u64 offset;
};

b32 isPowerOfTwo(u64 x) { return (x & (x - 1)) == 0; }

void *memAlignForward(void *ptr, u64 alignment) {
  Assert(isPowerOfTwo(alignment));
  u64 p = (u64)ptr;
  u64 a = alignment;

  u64 modulo = p & (a - 1);
  if (modulo != 0) {
    p += a - modulo;
  }
  return (void *)p;
}

void *arenaAllocAligned(Arena *arena, u64 size) {
  Assert(arena->offset + size <= arena->len);
  void *newMem = (void *)&arena->buffer[arena->offset];
  newMem = memAlignForward(newMem, 64);
  memset(newMem, 0, size);
  arena->offset += size;
  return newMem;
}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#endif

void *arenaAlloc(Arena *arena, u64 size) {
  return arenaAllocAligned(arena, DEFAULT_ALIGNMENT);
}

Arena arenaInit(u8 *backingBuffer, u64 len) {
  return (Arena){
      .buffer = backingBuffer,
      .len = len,
      .offset = 0,
  };
}

void arenaFreeAll(Arena *arena) { arena->offset = 0; }

typedef struct Stack Stack;
struct Stack {
  u8 *Buffer;
  u64 len;
  u64 offset;
};

typedef struct StackHeader StackHeader;
struct StackHeader {
  u8 padding;
};

Stack stackInit(u8 *backingBuffer, u64 len) {
  return (Stack){
      .Buffer = backingBuffer,
      .len = len,
      .offset = 0,
  };
}

u64 paddingWithHeader(void *mem, u64 alignment) {
  Assert(isPowerOfTwo(alignment));
  u64 p = (u64)mem;
  u64 a = alignment;
  u64 modulo = p & (a - 1);
  u64 padding = 0;
  if (modulo != 0) {
    padding = a - modulo;
  }
  u64 headerSpace = sizeof(StackHeader);
  if (padding < headerSpace) {
    headerSpace -= padding;
    if ((headerSpace & (a - 1)) != 0) {
      // NOTE : off by 1 erros if the alignment is unpair
      padding += a * (1 + (headerSpace / a));
    } else {
      padding += a * (headerSpace / a);
    }
  }
  return padding;
}

void *stackAllocAlign(Stack *stack, u64 size, u64 alignment) {
  if (alignment > 128) {
    alignment = 128;
  }
  StackHeader header;
  void *currAdress = (void *)(stack->Buffer + stack->offset);
  u64 padding = paddingWithHeader(newMem, alignment);
  Assert(size + padding + stack->offset <= stack->len);
  stack->offset += padding;

  header = padding;
  return newMem;
}

void *stackAlloc(Stack *stack, u64 size) {
  return stackAllocAlign(stack, size, DEFAULT_ALIGNMENT);
}

i32 main() {
  i32 x = 48;
  void *p = &x;
  printf("p : %p\n", p);
  p = memAlignForward(p, 64);
  printf("p : %p\n", p);
  return 0;
}
