#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <applock.h>

// Hash function for strings
unsigned int hash(char* key, size_t size) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash * 31) + *key;
        key++;
    }
    return hash % size;
}

// Initialize the map
void init_applock_map(AppLock* map, size_t size) {
    map->size = size;
    map->table = (Node**)calloc(size, sizeof(Node*));
    if (map->table == NULL) {
        perror("Error creating map table");
        exit(EXIT_FAILURE);
    }

    return map;
}

// Insert a key-value pair into the map
void insert_applock(AppLock* map, char* key) {
    unsigned int index = hash(key, map->size);

    // Check if the key already exists
    Node* current = map->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Key already exists, do not insert
            return;
        }
        current = current->next;
    }

    // Create a new node
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Error creating node");
        exit(EXIT_FAILURE);
    }

    newNode->key = strdup(key);
    pthread_mutex_init(&newNode->mutex, NULL);
    newNode->next = NULL;

    // Insert the new node into the hash table
    if (map->table[index] == NULL) {
        map->table[index] = newNode;
    } else {
        // Collision handling: insert at the beginning of the linked list
        newNode->next = map->table[index];
        map->table[index] = newNode;
    }
}

// Lock the mutex associated with a key
void lock_app(AppLock* map, char* key) {
    unsigned int index = hash(key, map->size);
    Node* current = map->table[index];

    // Traverse the linked list at the hash table index
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            pthread_mutex_lock(&current->mutex);
            return;
        }
        current = current->next;
    }

    // Key not found
    fprintf(stderr, "Key not found: %s\n", key);
    exit(EXIT_FAILURE);
}

// Unlock the mutex associated with a key
void unlock_app(AppLock* map, char* key) {
    unsigned int index = hash(key, map->size);
    Node* current = map->table[index];

    // Traverse the linked list at the hash table index
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            pthread_mutex_unlock(&current->mutex);
            return;
        }
        current = current->next;
    }

    // Key not found
    fprintf(stderr, "Key not found: %s\n", key);
    exit(EXIT_FAILURE);
}
