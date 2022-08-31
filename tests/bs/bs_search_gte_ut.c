#include "tests/base/utils.h"

#include "tree/spec/gbs.h"
#include "tree/rb.h"

int main() {
  gbs_tree_t t = gbs_create_tree(NULL, gtype_cmp_l, NULL);
  rb_insert(t, gtype_l(1));
  rb_insert(t, gtype_l(3));
  rb_insert(t, gtype_l(5));
  rb_insert(t, gtype_l(7));
  rb_insert(t, gtype_l(9));
  CHECK_MSG(gbs_search_gte(t, gtype_l(10)) == NULL, "Tìm >= 10");
  CHECK_MSG(gbs_search_gte(t, gtype_l(2))->key.l == 3, "Tìm >= 2");
  CHECK_MSG(gbs_search_gte(t, gtype_l(5))->key.l == 5, "Tìm >= 5");
  gbs_free_tree(t);
  TEST_OK();
  return 0;
}
