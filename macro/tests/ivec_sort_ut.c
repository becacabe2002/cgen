#include "algo.h"
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

VEC_DECL_IMPL(ivec, int)

int icmp(const void *o1, const void *o2) {
  return *((const int *)o1) - *((const int *)o2);
}

int main() {
  srand(time(NULL));
  struct ivec *v = ivec(0);
  for (int i = 0; i < 10; ++i) {
    ivec_append(v, rand());
  }
  VQ2INSORT(v, icmp);
  for (int i = 0; i < v->size; ++i) {
    if (i > 0 && icmp(v->elems + i - 1, v->elems + i) > 0) {
      return 1;
    }
  }
  printf("sz: %d\ncap: %d\n", v->size, v->cap);
  for (int i = 0; i < v->size; ++i) {
    printf("%d\n", v->elems[i]);
  }
  ivec_free(v);
}