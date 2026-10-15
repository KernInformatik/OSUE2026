#include <stdio.h>
#include <stdlib.h>

struct vec {
  float x;
  float y;
};

// struct vec *plus(vec *a, vec *b) {
//   struct vec *r = malloc(sizeof(struct vec));
//   r->x = a->x + b->x;
//   r->y = a->y + b->y;
//   return r;
// }

/**
 * This makes more sense, since a vecto is theoretically a new object in Memory
 * however, the pointer implementation of it is below this function
 */
struct vec addVector(struct vec *a, struct vec *b) {
  struct vec rv;
  rv.x = a->x + b->x;
  rv.y = a->y + b->y;
  return rv;
}

struct vec *plus(struct vec *a, struct vec *b) {
  struct vec *rv = (struct vec *)malloc(sizeof(struct vec));

  rv->x = a->x + b->x;
  rv->y = a->y + b->y;

  // needs to be freed where the function is called
  return rv;
}

// int main(int argc, char *argv[]) {
//   struct vec a = {12.34, 45.67};
//   struct vec b = {89.1, 1011.12};

//   struct vec *aPlusB = plus(a, b);
//   free(aPlusB);
//   printf("a + b = { x: %f, y: %f }\n", aPlusB->x, aPlusB->y);
// }

int main(int argc, char **argv) {
  struct vec a = {.x = 12.34, .y = 45.67};

  struct vec b = {.x = 89.10, .y = 1011.12};

  struct vec *newVec = plus(&a, &b);
  printf("a + b = { x: %f, y: %f }\n", newVec->x, newVec->y);
  free(newVec);

  struct vec Vector = addVector(&a, &b);
  struct vec *ptrToVec = &Vector;
  printf("a + b = { x: %f, y: %f }\n", ptrToVec->x, ptrToVec->y);

  return 0;
}
