/* (C) Nguyễn Bá Ngọc 2024-2025 */
#ifndef HMAP_H_
#define HMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum hmap_elem_state {
  UNUSED = 0,
  USING,
  DELETED,
  STATE_COUNT
};

const int prime_mod [] =
{
  1,          /* 1 << 0 */
  2,
  3,
  7,
  13,
  31,
  61,
  127,
  251,
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,      /* 1 << 16 */
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647  /* 1 << 31 */
};

#define HASH_MIN_SHIFT 3

#define HASH2IDX(hm, hash)(((hash) * 11) % (hm)->mod)

#define GET_BIT(bitmap, index) (((bitmap)[(index) / 8] >> ((index) % 8)) & 1)
#define SET_BIT(bitmap, index) ((bitmap)[(index) / 8] |= 1U << ((index) % 8))

#define HMAP_DECL(hname, key_t, value_t) \
struct hname##_elem; \
struct hname; \
struct hname *hname(unsigned (*ha)(const key_t), \
                    int (*cmp)(const key_t, const key_t)); \
struct hname *hname##_fk(struct hname* hm, void (*fk)()); \
struct hname *hname##_fv(struct hname* hm, void (*fv)()); \
struct hname *hname##_ak(struct hname *hm, void (*ak)(key_t *des, const key_t src)); \
struct hname *hname##_av(struct hname *hm, void (*av)(value_t *des, const value_t src)); \
struct hname *hname##_realloc(struct hname *hm, int ncap); \
struct hname##_elem *hname##_put(struct hname *hm, key_t k, value_t v); \
struct hname##_elem *hname##_get(struct hname *hm, const key_t k); \
int hname##_rem(struct hname *hm, const key_t k); \
struct hname##_elem *hname##_first(struct hname *hm); \
struct hname##_elem *hname##_next(struct hname *hm, struct hname##_elem *n); \
void hname##_trav(struct hname *hm, void (*f)(struct hname##_elem *n, void *u), void *u); \
void hname##_free(struct hname *hm);

#define HMAP_IMPL(hname, key_t, value_t) \
struct hname##_elem { \
  key_t key; \
  value_t value; \
  unsigned hash; \
  enum hmap_elem_state state; \
}; \
struct hname { \
  struct hname##_elem *elems; \
  struct hname##_elem *end; \
  int mod; \
  unsigned mask; \
  int size; \
  int cap; \
  int used; \
  unsigned (*ha)(const key_t); \
  int (*cmp)(const key_t, const key_t); \
  void (*fk)(); \
  void (*fv)(); \
  void (*ak)(key_t *des, const key_t src); \
  void (*av)(value_t *des, const value_t src); \
}; \
static int closest_shift(int n) { \
  int i = 0; \
  for (; n; ++i) { \
    n >>= 1; \
  } \
  return i; \
} \
static void hname##_set_shift(struct hname *hm, int shift) { \
  hm->cap = 1 << shift; \
  hm->mod = prime_mod[shift]; \
  hm->mask = hm->cap - 1; \
} \
static void hname##_set_shift_from_cap(struct hname *hm, int cap) { \
  int shift = closest_shift(cap); \
  if (shift < HASH_MIN_SHIFT) { \
    shift = HASH_MIN_SHIFT; \
  } \
  hname##_set_shift(hm, shift); \
} \
static void hname##_setup(struct hname *hm, int cap) { \
  int shift = closest_shift(cap); \
  if (shift < HASH_MIN_SHIFT) { \
    shift = HASH_MIN_SHIFT; \
  } \
  hname##_set_shift(hm, shift); \
  hm->elems = calloc(hm->cap, sizeof(struct hname##_elem)); \
  hm->end = hm->elems + hm->cap; \
} \
static void hname##_realloc_arrays(struct hname *hm) { \
  hm->elems = realloc(hm->elems, hm->cap * sizeof(struct hname##_elem)); \
  hm->end = hm->elems + hm->cap; \
} \
static void relocate_map(struct hname *hm, unsigned ocap, \
          unsigned char *reallocated_flags) { \
  for (int i = 0; i < ocap; ++i) { \
    if (hm->elems[i].state != USING) { \
      hm->elems[i].state = UNUSED; \
      continue; \
    } \
    if (GET_BIT(reallocated_flags, i)) { \
      continue; \
    } \
    struct hname##_elem n = hm->elems[i]; \
    hm->elems[i].state = UNUSED; \
    for (;;) { \
      unsigned idx, step = 0; \
      idx = HASH2IDX(hm, n.hash); \
      while (GET_BIT(reallocated_flags, idx)) { \
        ++step; \
        idx += step; \
        idx &= hm->mask; \
      } \
      SET_BIT(reallocated_flags, idx); \
      if (hm->elems[idx].state != USING) { \
        hm->elems[idx] = n; \
        break; \
      } \
      struct hname##_elem tmp = hm->elems[idx]; \
      hm->elems[idx] = n; \
      n = tmp; \
    } \
  } \
} \
struct hname *hname##_realloc(struct hname *hm, int ncap) { \
  if (ncap == hm->cap || ncap <= hm->size) { \
    return hm; \
  } \
  int ocap = hm->cap; \
  hname##_set_shift_from_cap(hm, ncap); \
  if (hm->cap > ocap) { \
    hname##_realloc_arrays(hm); \
    memset(hm->elems + ocap, 0, (hm->cap - ocap) * sizeof(struct hname##_elem)); \
  } \
  int maxcap = ocap > hm->cap? ocap: hm->cap; \
  unsigned char *reallocated_flags = calloc((maxcap + 7) / 8, sizeof(unsigned char)); \
  relocate_map(hm, ocap, reallocated_flags); \
  free(reallocated_flags); \
  if (hm->cap < ocap) { \
    hname##_realloc_arrays(hm); \
  } \
  hm->used = hm->size; \
  return hm; \
} \
static inline int hname##_maybe_realloc(struct hname *hm) { \
  unsigned size = hm->size, used = hm->used, cap = hm->cap; \
  if (1.1 * size >= cap - 1 || \
      (cap > 1024 && cap > 3 * size && used > 2 * size)) { \
    hname##_realloc(hm, hm->size * 1.333); \
    return 1; \
  } \
  return 0; \
} \
static struct hname##_elem *hname##_rem_elem(struct hname *hm, int idx) { \
  struct hname##_elem *n = hm->elems + idx; \
  if (hm->fk) { \
    hm->fk(n->key); \
  } \
  if (hm->fv) { \
    hm->fv(n->value); \
  } \
  n->state = DELETED; \
  hm->size--; \
  return n; \
} \
static inline int hname##_lookup(struct hname *hm, const key_t key, \
      unsigned *hash_return) { \
  unsigned lookup_hash = hm->ha(key); \
  if (hash_return) { \
    *hash_return = lookup_hash; \
  } \
  int idx = HASH2IDX(hm, lookup_hash); \
  struct hname##_elem *n = hm->elems + idx; \
  int first_deleted = -1; \
  int step = 0; \
  while (n->state != UNUSED) { \
    if (n->state == USING) { \
      if (n->hash == lookup_hash && hm->cmp(n->key, key) == 0) { \
        return idx; \
      } \
    } else if (n->state == DELETED && first_deleted < 0) { \
      first_deleted = idx; \
    } \
    ++step; \
    idx += step; \
    idx &= hm->mask; \
    n = hm->elems + idx; \
  } \
  if (first_deleted >= 0) { \
    return first_deleted; \
  } \
  return idx; \
} \
struct hname *hname(unsigned (*ha)(const key_t), \
                    int (*cmp)(const key_t, const key_t)) { \
  struct hname *hm = malloc(sizeof(struct hname)); \
  hm->size = 0; \
  hm->used = 0; \
  hm->ha = ha; \
  hm->cmp = cmp; \
  hm->fk = 0; \
  hm->fv = 0; \
  hm->ak = 0; \
  hm->av = 0; \
  hname##_setup(hm, 0); \
  return hm; \
} \
struct hname *hname##_fk(struct hname* hm, void (*fk)()) { \
  if (!hm) { \
    return hm; \
  } \
  hm->fk = fk; \
  return hm; \
} \
struct hname *hname##_fv(struct hname* hm, void (*fv)()) { \
  if (!hm) { \
    return hm; \
  } \
  hm->fv = fv; \
  return hm; \
} \
struct hname *hname##_ak(struct hname *hm, void (*ak)(key_t *des, const key_t src)) { \
if (!hm) { \
    return hm; \
  } \
  hm->ak = ak; \
  return hm; \
} \
struct hname *hname##_av(struct hname *hm, void (*av)(value_t *des, const value_t src)) { \
  if (!hm) { \
    return hm; \
  } \
  hm->av = av; \
  return hm; \
} \
struct hname##_elem *hname##_put(struct hname *hm, key_t k, value_t v) { \
  unsigned key_hash; \
  int idx = hname##_lookup(hm, k, &key_hash); \
  struct hname##_elem *n = hm->elems + idx; \
  if (n->state == USING) { \
    return n; \
  } \
  n->hash = key_hash; \
  if (hm->ak) { \
    hm->ak(&n->key, k); \
  } else { \
    n->key = k; \
  } \
  if (hm->av) { \
    hm->av(&n->value, v); \
  } else { \
    n->value = v; \
  } \
  ++hm->size; \
  int new_usage = n->state == UNUSED; \
  n->state = USING; \
  if (new_usage) { \
    ++hm->used; \
    hname##_maybe_realloc(hm); \
  } \
  return NULL; \
} \
struct hname##_elem *hname##_get(struct hname *hm, const key_t k) { \
  int idx = hname##_lookup(hm, k, NULL); \
  struct hname##_elem *n = hm->elems + idx; \
  if (n->state == USING) { \
    return n; \
  } \
  return NULL; \
} \
int hname##_rem(struct hname *hm, const key_t key) { \
  int idx = hname##_lookup(hm, key, NULL); \
  if (hm->elems[idx].state != USING) { \
    return 0; \
  } \
  hname##_rem_elem(hm, idx); \
  if (hname##_maybe_realloc(hm)) { \
    idx = hname##_lookup(hm, key, NULL); \
  } \
  return 1; \
} \
void hname##_free(struct hname *hm) { \
  for (struct hname##_elem *iter = hm->elems; iter < hm->end; ++iter) { \
    if (iter->state == USING) { \
      if (hm->fk) { \
        hm->fk(iter->key); \
      } \
      if (hm->fv) { \
        hm->fv(iter->value); \
      } \
    } \
  } \
  free(hm->elems); \
  free(hm); \
} \
struct hname##_elem *hname##_first(struct hname *hm) { \
  for (struct hname##_elem *iter = hm->elems; iter < hm->end; ++iter) { \
    if (iter->state == USING) { \
      return iter; \
    } \
  } \
  return NULL; \
} \
struct hname##_elem *hname##_next(struct hname *hm, struct hname##_elem *n) { \
  ++n; \
  while (n < hm->end) { \
    if (n->state == USING) { \
      return n; \
    } \
    ++n; \
  } \
  return NULL; \
} \
void hname##_trav(struct hname *hm, void (*f)(struct hname##_elem *n, void *u), void *u) { \
  for (struct hname##_elem *n = hname##_first(hm); n; n = hname##_next(hm, n)) { \
      f(n, u); \
  } \
} \

#define HMAP_DECL_IMPL(hname, key_t, value_t) \
HMAP_DECL(hname, key_t, value_t) \
HMAP_IMPL(hname, key_t, value_t)

static inline unsigned hash_s(const char *s) {
  register unsigned h = 5381;
  for (; *s != '\0'; ++s) {
    h = (h << 5) + h + *s;
  }
  return h;
}

#endif  // HMAP_H_
