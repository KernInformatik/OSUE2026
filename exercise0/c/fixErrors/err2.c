#include <stdio.h>

int main(int argc, char **argv) {
  char *p = "Hello\n";
  char **p2 = &p;
  printf("%d\n",**p2);
  return 0;
}
