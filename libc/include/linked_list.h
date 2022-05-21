#pragma once

struct linked_list_member {
  struct linked_list_member* next;
  struct linked_list_member* prev;
  void* node;
};
struct linked_list {
  struct linked_list_member* first;
  struct linked_list_member* last;
};

void insert_linked_list(struct linked_list* list, struct linked_list_member* new_member, void* node);
void remove_linked_list(struct linked_list* list, struct linked_list_member* remove_member);
