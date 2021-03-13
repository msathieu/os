#include <linked_list.h>
#include <stdlib.h>

void insert_linked_list(struct linked_list* list, struct linked_list_member* new_member) {
  new_member->next = list->first;
  list->first = new_member;
}
void remove_linked_list(struct linked_list* list, struct linked_list_member* remove_member) {
  struct linked_list_member* prev_member = 0;
  for (struct linked_list_member* member = list->first; member; member = member->next) {
    if (member == remove_member) {
      if (prev_member) {
        prev_member->next = remove_member->next;
      } else {
        list->first = remove_member->next;
      }
      return;
    }
    prev_member = member;
  }
  exit(1);
}
