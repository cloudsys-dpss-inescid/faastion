#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

struct linked_list_elem {
    struct linked_list_elem *next;
    void *item;
};

struct linked_list {
    struct linked_list_elem *head;
};

struct hash_table_entry {
    unsigned long key;
    void *item;
};

struct hash_table {
    struct linked_list **buckets;
    int size;
    int count;
    void (*destroy_entry)(void *);
};

struct hash_table *new_hash_table(int size);
void *hash_table_lookup(struct hash_table *ht, unsigned long key);
void hash_table_insert(struct hash_table *ht, unsigned long key, void *item);
void hash_table_remove(struct hash_table *ht, unsigned long key, void (*handle)(void *));
void hash_table_destroy(struct hash_table *ht, void (*handle)(void *));

extern struct hash_table *proc_tbl;

#endif