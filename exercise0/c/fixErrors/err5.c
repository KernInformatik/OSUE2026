#include <stdio.h>
#include <stdlib.h>

struct vec {
  float x;
  float y;
};

struct vec *plus(vec *a, vec *b) {
  struct vec *r = malloc(sizeof (struct vec));
  r->x = a->x + b->x;
  r->y = a->y + b->y;
  return r;
}

int main (int argc, char* argv[]) {
  struct vec a = { 12.34, 45.67 };
  struct vec b = { 89.1,  1011.12 };

  struct vec *aPlusB = plus(a, b);
  free(aPlusB);
  printf("a + b = { x: %f, y: %f }\n", aPlusB->x, aPlusB->y);
}
