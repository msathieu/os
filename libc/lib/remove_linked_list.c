#include <linked_list.h>

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
