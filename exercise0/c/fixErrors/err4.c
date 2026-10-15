#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  printf("Integers from 1 to 23:\n");
  uint64_t ints[] = {0x07060504030201, 0x0f0e0d0c0b0a0908, 0x1716151413121110};
  for (size_t i = 0; i <= 23; i++) // shit goes out of fucking bound?
  {
    printf("  %d\n",
           ((uint8_t *)ints)[i]); // why the down cast EVEN INTO A POINTER TYPE?
                                  // You will loose bits!
  }

  /*A more sane way would be this*/
  uint64_t *ptrToInts = &ints[0];

  /* I wanna fuck every vibecoder in the ass */
  while (*(uint64_t *)ptrToInts <= ints[2]) {
    printf(" %d", *(uint8_t *)ptrToInts);
    printf(" Sizeof of *ptrToInts: %zu\n Sizeof the datatype uint8_t* %zu",
           sizeof(*ptrToInts), sizeof(uint8_t *));
    ptrToInts += 1;
  }

  free(ptrToInts);
  return 0;
}
