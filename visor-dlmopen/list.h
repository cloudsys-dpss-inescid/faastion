#ifndef __LIST_H__
#define __LIST_H__

typedef struct List List; // private fields

List *new_list();
int get_size(List *list);
void add_node(List *list, pid_t pid);
void remove_node(List *list, pid_t pid);
void print_list(List *list);
int lookup_node(List *list, pid_t pid);

#endif // __LIST_H__