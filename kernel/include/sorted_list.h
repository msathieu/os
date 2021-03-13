#pragma once
#include <linked_list.h>

typedef int (*sorted_list_compare)(void*, void*);

struct sorted_list {
  struct linked_list_member* first;
  sorted_list_compare compare;
};

void insert_sorted_list(struct sorted_list*, struct linked_list_member*);
static inline void remove_sorted_list(struct sorted_list* list, struct linked_list_member* remove_member) {
  remove_linked_list((struct linked_list*) list, remove_member);
}
