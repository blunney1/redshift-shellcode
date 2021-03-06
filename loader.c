#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

// Simple-1: gcc -c -O simple-1.c
/* char shellcode[] = "\xb8\x2a\x00\x00\x00\xc3"; */
/* int offset = 0; */
// Simple-2: gcc -c -O simple-2.c
/* char shellcode[] = "\xb8\x01\x00\x00\x00\x85\xff\x74\x0f\x53\x89\xfb\x8d\x7f\xff\xe8\xec\xff\xff\xff\x0f\xaf\xc3\x5b\xf3\xc3\x41\x54\x55\x53\x85\xff\x7e\x20\x41\x89\xfc\xbb\x00\x00\x00\x00\xbd\x00\x00\x00\x00\x89\xdf\xe8\xca\xff\xff\xff\x01\xc5\x83\xc3\x01\x41\x39\xdc\x75\xef\xeb\x05\xbd\x00\x00\x00\x00\x89\xe8\x5b\x5d\x41\x5c\xc3"; */
/* int offset = 26; */

// Chess
#include "chess.bytes"

void print_hex(char *start, char *end)
{
  for (char c; start < end; start++) {
    c = *start;
    fprintf(stderr, "%02hhx", c);
  }
  putchar('\n');
}


typedef struct gamestate {
  uint64_t rooks;
  uint64_t knights;
  uint64_t bishops;
  uint64_t queens;
  uint64_t kings;
  uint64_t pawns;
  union {
    uint64_t player;
    uint64_t current_piece_bb; // For iterators
  };
  int en_passant_sq;
  union {
    uint64_t castle_flags;
    uint64_t promotion_piece; // For iterators
  };
} gamestate;

typedef void (*shellcode_t)(const char*, char*);
void invoke(shellcode_t f, const char* g, char* m) { f(g, m); }

extern "C" uint64_t custom_main(gamestate);

int main() {
  int size = sizeof(shellcode)-1;
  char *src_ptr = shellcode;
  void *code_ptr = valloc(size);
  printf("Codebase: %08x\n", code_ptr);
  memmove(code_ptr, src_ptr, size);
  int err = mprotect(code_ptr, size, 0x7);
  if (0 != err) {
    fprintf(stderr, "mprotect: %d\n", err);
    return 1;
  }
  shellcode_t f = (shellcode_t)(code_ptr+offset);
  /* print_hex(f, code_ptr + size); */
  const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
  char move_buf[10];
  invoke(f, fen, move_buf);
  printf("%s\n", move_buf);
  free(code_ptr);

  
  return 0;
}
