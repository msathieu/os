#include <linked_list.h>
#include <panic.h>

// Scheduler: new entries must be added at the end
void insert_linked_list(struct linked_list* list, struct linked_list_member* new_member) {
  new_member->next = 0;
  if (!list->first) {
    list->first = new_member;
    return;
  }
  struct linked_list_member* member = list->first;
  while (member->next) {
    member = member->next;
  }
  member->next = new_member;
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
  panic("Can't remove member that isn't already in the list");
}
