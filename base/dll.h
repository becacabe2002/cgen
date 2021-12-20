#ifndef BASE_DLL_H_
#define BASE_DLL_H_

/*
  (c) Nguyen Ba Ngoc, 2021
  Generic double Linked List (DLL/dll)
*/

typedef struct dll_node_s {
  struct dll_node_s *next;
  struct dll_node_s *prev;
} *dll_node_t;

typedef struct dll_s {
  dll_node_t front;
  dll_node_t back;
} *dll_t;

#define to_dll_node(n) ((dll_node_t)n)
#define to_dll_list(list) ((dll_t)list)

#define dll_traverse(cur, list) \
  for (dll_node_t cur = list->front; cur != NULL; cur = cur->next)

#define dll_rtraverse(cur, list) \
  for (dll_node_t cur = list->back; cur != NULL; cur = cur->prev)

/* Giao diện khái quát */

dll_node_t dll_create_node();
dll_t dll_create_list();
void dll_free_list(dll_t list);

void dll_push_back(dll_t list, dll_node_t nn);
void dll_push_front(dll_t list, dll_node_t nn);
void dll_pop_front(dll_t list);
void dll_pop_back(dll_t list);
dll_node_t dll_front(dll_t list);
dll_node_t dll_back(dll_t list);
dll_node_t dll_inserta(dll_t list, dll_node_t pos, dll_node_t nn);
dll_node_t dll_insertb(dll_t list, dll_node_t pos, dll_node_t nn);
int dll_is_empty(dll_t list);
long dll_length(dll_t list);
void dll_erase(dll_t list, dll_node_t pos);
void dll_clear(dll_t list);

void dll_pprint_node(dll_node_t);
void dll_pprint_list(dll_t);
#define dll_free_node(n) free(n)

#endif  // BASE_DLL_H_