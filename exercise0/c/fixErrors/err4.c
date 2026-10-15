#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("Integers from 1 to 23:\n");
  uint64_t ints[] = {0x07060504030201, 0x0f0e0d0c0b0a0908, 0x1716151413121110};
  for (size_t i = 0; i <= 23; i++) {
    printf("  %d\n", ((uint8_t*) ints)[i]);
  }
  return 0;
}
