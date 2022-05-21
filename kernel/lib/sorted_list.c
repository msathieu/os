#include <sorted_list.h>

void insert_sorted_list(struct sorted_list* list, struct linked_list_member* new_member, void* node) {
  new_member->node = node;
  new_member->prev = 0;
  new_member->next = 0;
  if (list->last && list->compare(new_member->node, list->last->node) >= 0) {
    new_member->prev = list->last;
    list->last->next = new_member;
    list->last = new_member;
  }
  struct linked_list_member* prev_member = 0;
  for (struct linked_list_member* member = list->first; member; member = member->next) {
    if (list->compare(new_member->node, member->node) < 0) {
      if (prev_member) {
        prev_member->next = new_member;
        new_member->prev = prev_member;
      } else {
        list->first = new_member;
      }
      new_member->next = member;
      member->prev = new_member;
      return;
    }
    prev_member = member;
  }
  if (prev_member) {
    prev_member->next = new_member;
    new_member->prev = prev_member;
    list->last = new_member;
  } else {
    list->first = new_member;
    list->last = new_member;
  }
}
void remove_sorted_list(struct sorted_list* list, struct linked_list_member* remove_member) {
  remove_linked_list((struct linked_list*) list, remove_member);
}
