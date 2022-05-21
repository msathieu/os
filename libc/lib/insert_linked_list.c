#include <linked_list.h>

void insert_linked_list(struct linked_list* list, struct linked_list_member* new_member, void* node) {
  new_member->node = node;
  new_member->prev = list->last;
  new_member->next = 0;
  if (list->last) {
    list->last->next = new_member;
  }
  list->last = new_member;
  if (!list->first) {
    list->first = new_member;
  }
}
