#include <stdio.h>

int main(int argc, char **argv) {
  char *p = "Hello\n";
  char **p2 = &p;
  printf(**p2);
  return 0;
}
