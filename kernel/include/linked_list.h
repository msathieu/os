#pragma once

struct linked_list_member {
  struct linked_list_member* next;
};
struct linked_list {
  struct linked_list_member* first;
};

void insert_linked_list(struct linked_list*, struct linked_list_member*);
void remove_linked_list(struct linked_list*, struct linked_list_member*);
