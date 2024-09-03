#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    pid_t pid;
    struct Node* next;
} Node;

Node* create_node(pid_t pid) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->pid = pid;
    newNode->next = NULL;
    return newNode;
}

void add_node(Node** head, pid_t pid) {
    Node* newNode = create_node(pid);
    newNode->next = *head;
    *head = newNode;
}

void remove_node(Node** head, pid_t pid) {
    Node* current = *head;
    Node* prev = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (prev == NULL) {
                // Node to be removed is the head
                *head = current->next;
            } else {
                // Node to be removed is in the middle or end
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

Node *lookup_node(Node *current, pid_t pid) {
    while (current != NULL) {
        if (current->pid == pid) {
            break;
        }
        current = current->next;
    }
    return current;
}

void print_list(Node* head) {
    Node* current = head;
    while (current != NULL) {
        printf("PID: %d\n", current->pid);
        current = current->next;
    }
}