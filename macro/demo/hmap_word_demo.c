#include "hmap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

HMAP_DECL_IMPL(hmap, char *, int)

void str_assign(char **des, const char *src) {
  *des = strdup(src);
}

int main() {
  struct hmap *hm = hmap(hash_s, strcmp);
  hmap_ak(hm, str_assign);
  hmap_fk(hm, free);
  char cmd[32], word[32];
  while (scanf("%s", cmd) == 1) {
    if (strcmp(cmd, "print") == 0) {
      printf("Traverse: \n");
      for (struct hmap_elem *e = hmap_first(hm); e; e = hmap_next(hm, e)) {
        printf("%s: %d\n", e->key, e->value);
      }
      continue;
    }
    scanf("%s", word);
    if (strcmp(cmd, "insert") == 0) {
      struct hmap_elem *e = hmap_put(hm, word, 1);
      if (e) {
        ++e->value;
        printf("freq(%s) updated = %d\n", word, e->value);
      } else {
        printf("Inserted %s\n", word);
      }
    } else if (strcmp(cmd, "freq") == 0) {
      struct hmap_elem *e = hmap_get(hm, word);
      printf("freq(%s) = %d\n", word, e? e->value: 0);
    } else if (strcmp(cmd, "rem") == 0) {
      if (hmap_rem(hm, word)) {
        printf("Deleted %s\n", word);
      }
    }
  }
  hmap_free(hm);
}