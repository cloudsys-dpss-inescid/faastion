#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>

struct hash_table *proc_tbl;

// Linked List

static struct linked_list *new_linked_list() {
    struct linked_list *ll = (struct linked_list *)malloc(sizeof(struct linked_list));
    ll->head = NULL;
    return ll;
}

static void linked_list_add(struct linked_list *ll, void *item) {
    struct linked_list_elem *elem = ll->head;
    struct linked_list_elem *new_elem = (struct linked_list_elem *)malloc(sizeof(struct linked_list_elem));
    new_elem->item = item;
    ll->head = new_elem;
    ll->head->next = elem;
}

static void linked_list_destroy(struct linked_list *ll, void (*handle)(void *)) {
    struct linked_list_elem *temp = NULL;
    struct linked_list_elem *p = ll->head;
    while (p != NULL) {
        temp = p;
        p = temp->next;
        if (handle)
            handle(temp->item);
        free(temp);
    }
    free(ll);
}

// Hash Table

static inline unsigned int hash(struct hash_table *ht, unsigned long key) {
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    return (unsigned int)(key % ht->size);
}

static inline struct hash_table_entry *bucket_lookup(struct linked_list *bucket, unsigned long key) {
    struct hash_table_entry *entry = NULL;
    struct linked_list_elem *p = bucket->head;
    for (; p != NULL; p = p->next) {
        if (((struct hash_table_entry *)p->item)->key == key) {
            entry = (struct hash_table_entry *)p->item;
            break;
        }
    }
    return entry;
}

struct hash_table *new_hash_table(int size) {
    struct hash_table *ht = (struct hash_table *)malloc(sizeof(struct hash_table));
    ht->buckets = (struct linked_list **)malloc(sizeof(struct linked_list *)*size);
    ht->size = size;
    for (int i = 0; i < size; i++) {
        ht->buckets[i] = new_linked_list();
    }
    return ht;
}

static void bucket_insert(struct linked_list *bucket, unsigned long key, void *item) {
    struct hash_table_entry *entry;
    entry = bucket_lookup(bucket, key);
    if (entry == NULL) {
        entry = (struct hash_table_entry *)malloc(sizeof(struct hash_table_entry));
        entry->key = key;
        entry->item = item;
        linked_list_add(bucket, entry);
    } else {
        entry->item = item;
    }
}

void hash_table_insert(struct hash_table *ht, unsigned long key, void *item) {
    struct linked_list *bucket = ht->buckets[hash(ht, key)];
    bucket_insert(bucket, key, item);
}

void *hash_table_lookup(struct hash_table *ht, unsigned long key) {
    struct linked_list *bucket = ht->buckets[hash(ht, key)];
    struct hash_table_entry *entry = bucket_lookup(bucket, key);
    return entry == NULL ? NULL : entry->item;
}

static void bucket_remove(struct linked_list *bucket, unsigned long key, void (*handle)(void *)) {
    struct linked_list_elem **p = &bucket->head;
    struct linked_list_elem *temp;
    struct hash_table_entry *entry;
    for (; *p != NULL; p = &(*p)->next) {
        temp = *p;
        entry = (struct hash_table_entry *)temp->item;
        if (entry->key == key) {
            *p = temp->next;
            if (handle)
                handle(entry->item);
            free(entry);
            free(temp);
            break;
        }
    }
}

void hash_table_remove(struct hash_table *ht, unsigned long key, void (*handle)(void *)) {
    struct linked_list *bucket = ht->buckets[hash(ht, key)];
    bucket_remove(bucket, key, handle);
}

void hash_table_destroy(struct hash_table *ht, void (*handle)(void *)) {
    for (int i = 0; i < ht->size; i++) {
        linked_list_destroy(ht->buckets[i], handle);
    }
    free(ht->buckets);
    free(ht);
}