#ifndef __LIST_H__
#define __LIST_H__

typedef struct Node Node; // private fields

void add_node(Node **head, pid_t pid);
void remove_node(Node **head, pid_t pid);
void print_list(Node *head);
Node *lookup_node(Node *current, pid_t pid);

#endif // __LIST_H__