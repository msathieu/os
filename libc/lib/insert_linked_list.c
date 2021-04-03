#include <linked_list.h>

void insert_linked_list(struct linked_list* list, struct linked_list_member* new_member) {
  new_member->next = list->first;
  list->first = new_member;
}
