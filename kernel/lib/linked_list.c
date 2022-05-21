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
void remove_linked_list(struct linked_list* list, struct linked_list_member* remove_member) {
  remove_member->node = 0;
  if (list->first == remove_member) {
    list->first = remove_member->next;
  }
  if (list->last == remove_member) {
    list->last = remove_member->prev;
  }
  if (remove_member->prev) {
    remove_member->prev->next = remove_member->next;
  }
  if (remove_member->next) {
    remove_member->next->prev = remove_member->prev;
  }
  remove_member->prev = 0;
  remove_member->next = 0;
}
