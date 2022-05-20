#pragma once

struct linked_list_member {
  struct linked_list_member* next;
  void* node;
};
struct linked_list {
  struct linked_list_member* first;
};

void insert_linked_list(struct linked_list*, struct linked_list_member*, void*);
void remove_linked_list(struct linked_list*, struct linked_list_member*);
