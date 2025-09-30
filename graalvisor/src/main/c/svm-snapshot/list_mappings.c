#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "list_mappings.h"
#include "cr.h"

size_t bytes_to_pages(size_t bytes) {
    return bytes / getpagesize();
}

char permission(void* mapping_start, void* mapping_finish, char* mapping_perms, void* addr) {

    // Check if the requested address is within the mapping limits.
    if (addr < mapping_start || addr > mapping_finish) {
        err("warning, trying to get permission of page outside mapping %p\n", addr);
    }

    // Number of bytes after the start of the mapping.
    size_t bytes = (char*) addr - (char*) mapping_start;
    return mapping_perms[bytes_to_pages(bytes)];
}

void* repeated(void* mapping_start, void* mapping_finish, char* mapping_perms, void* block_start) {
    char  block_perm = permission(mapping_start, mapping_finish, mapping_perms, block_start);
    char* block_finish = block_start;
    for (; block_finish < (char*) mapping_finish; block_finish += getpagesize()) {
        if (permission(mapping_start, mapping_finish, mapping_perms, block_finish) != block_perm) {
            break;
        }
    }
    return block_finish;
}

void init_mapping(mapping_t* mapping, void* start, size_t size) {
    mapping->start = start;
    mapping->size = size;
    mapping->permissions_base = mapping->permissions = (char*) malloc(bytes_to_pages(size) * sizeof(char));
    mapping->dirty_base = mapping->dirty = (char*) malloc(bytes_to_pages(size) * sizeof(char));
    mapping->validated_base = mapping->validated = (char*) malloc(bytes_to_pages(size) * sizeof(char));
    memset(mapping->permissions, 0, bytes_to_pages(size));
    memset(mapping->dirty, 0, bytes_to_pages(size));
    memset(mapping->validated, 0, bytes_to_pages(size));
    mapping->next = NULL;
}

mapping_t* list_mappings_push(mapping_t* head, void* start, size_t size) {
    mapping_t* current = head;

    // If the list is empty, fill in the head and return.
    if (current->start == NULL) {
        init_mapping(current, start, size);
        return current;
    }

    // Otherwise, find te last element.
    while (current->next != NULL && current->next->start < start) {
        current = current->next;
    }

    // Insert new element after current.
    mapping_t * new = (mapping_t *) malloc(sizeof(mapping_t));
    init_mapping(new, start, size);
    new->next = current->next;
    current->next = new;
    return new;
}

void list_mappings_delete(mapping_t* head, mapping_t* to_delete) {
    // If the list is empty or the element to delete is null, do nothing.
    if (head->start == NULL || to_delete == NULL) {
        return;
    }
    // If the element to delete is the head, we copy the second element into the head and delete the second element.
    else if (head == to_delete) {
        // We need free the old dirty and permissions before we copy or memset.
        // If head node is the only element in the list.
        free(head->dirty_base);
        free(head->permissions_base);
        free(head->validated_base);
        if (head->next == NULL) {
            memset(head, 0, sizeof(mapping_t));
        } else {
            // Save pointer to the second element.
            to_delete = head->next;
            // Copy the second element into the head.
            memcpy(head, head->next, sizeof(mapping_t));
            // Free the second element.
            free(to_delete);
        }
        return;
    }
    // Else, iterate the list until you find the element to delete.
    else {
        mapping_t* current = head;
        // Iterate until we find the node just before the one to delete.
        while (current != NULL) {
            // If the element to delete is the next
            if (current->next == to_delete) {
                current->next = to_delete->next;
                free(to_delete->dirty_base);
                free(to_delete->permissions_base);
                free(to_delete->validated_base);
                free(to_delete);
                return;
            }
            current = current->next;
        }
    }

    err("warning: unable to delete mapping %16p - %16p\n", to_delete->start, ((char*) to_delete->start) + to_delete->size);
}

mapping_t* list_mappings_find(mapping_t* head, void* start, size_t size) {
    mapping_t* current = head;
    void* finish = ((char*) start) + size;

    // If the list if empty, nothing to print.
    if (current == NULL || current->start == NULL) {
        return NULL;
    }

    while (current != NULL) {
        void* current_finish = ((char*) current->start) + current->size;

        // If the range is included in this mapping.
        if (start >= current->start && finish <= current_finish) {
            return current;
        }
        // We do not support partial overlaps. Quit with error message.
        // Note that start is part of the range but the finish address is already outside.
        else if ((start >= current->start && start < current_finish) || (finish > current->start && finish <= current_finish)) {
            err("error: requested mapping %16p - %16p partially overlaps with an existing mapping %16p - %16p\n",
                start, finish, current->start, current_finish);
            return NULL;
        }
        // We do not support large mappings that include more than a single existing mapping.
        else if (start < current->start && finish > current_finish) {
            err("error: requested mapping %16p - %16p includes an existing mapping %16p - %16p\n",
                start, finish, current->start, current_finish);
            return NULL;
        }
        // Continue looping.
        else {
            current = current->next;
        }
    }

    return NULL;
}

char* array_merge(char* arr1, size_t arr1_len, char* arr2, size_t arr2_len) {
        size_t newsize = arr1_len + arr2_len;
        char* newarr = (char*) malloc(newsize * sizeof(char));
        memcpy(newarr, arr1, bytes_to_pages(arr1_len));
        memcpy(newarr + bytes_to_pages(arr1_len), arr2, bytes_to_pages(arr2_len));
        return newarr;
}

void list_mappings_merge(mapping_t* head) {
    mapping_t* current = head;

    if (current == NULL || current->start == NULL) {
        return;
    }

    while (current->next != NULL) {
        void* current_finish = ((char*) current->start) + current->size;

        // If the current finish matches the start of the next, merge.
        if (current_finish == current->next->start) {
            size_t csize = current->size;
            size_t nsize = current->next->size;
            current->dirty = array_merge(current->dirty, csize, current->next->dirty, nsize);
            current->permissions = array_merge(current->permissions, csize, current->next->permissions, nsize);
            current->validated = array_merge(current->validated, csize, current->next->validated, nsize);
            current->size += nsize;
            // Free the old buffers.
            free(current->dirty_base);
            free(current->permissions_base);
            free(current->validated_base);
            // Initialize the new base pointers with the buffers just allocated.
            current->dirty_base = current->dirty;
            current->permissions_base = current->permissions;
            current->validated_base = current->validated;
            list_mappings_delete(head, current->next);
        } else {
            current = current->next;
        }
    }
}

void print_block(void* block_start, void* block_finish, char block_perm) {
    dbg("pmapping: %16p - %16p size = 0x%16lx prot = %s%s%s%s\n",
        block_start,
        block_finish,
        (char*) block_finish - (char*) block_start,
        block_perm & PROT_EXEC   ? "x" : "-",
        block_perm & PROT_READ   ? "r" : "-",
        block_perm & PROT_WRITE  ? "w" : "-",
        block_perm == PROT_NONE  ? "n" : "-");
}

void print_mapping(void* mapping_start, void* mapping_finish, char* mapping_perms) {
    void* block_start = mapping_start;

    while (block_start < mapping_finish) {
        char  block_perm = permission(mapping_start, mapping_finish, mapping_perms, block_start);
        char* block_finish = repeated(mapping_start, mapping_finish, mapping_perms, block_start);
        print_block(block_start, block_finish, block_perm);
        block_start = block_finish;
    }
}

void print_list_mappings(mapping_t * head) {
    mapping_t* current = head;

    // If the list if empty, nothing to print.
    if (current->start == NULL) {
        return;
    }

    while (current != NULL) {
        void* mapping_start = current->start;
        void* mapping_finish = ((char*) mapping_start) + current->size;
        print_mapping(mapping_start, mapping_finish, current->permissions);
        current = current->next;
    }
}

void mapping_update_validated(mapping_t* mapping, void* block_start, void* block_finish) {
    // First page index;
    int i = bytes_to_pages((char*) block_start - (char*) mapping->start);
    // Last page index.
    int j = bytes_to_pages((char*) block_finish - (char*) mapping->start);
    // Update validated between pages i and f.
    memset(mapping->validated + i, 1, j - i);
}

void mapping_update_permissions(mapping_t* mapping, void* block_start, void* block_finish, char block_perm) {
    // First page index;
    int i = bytes_to_pages((char*) block_start - (char*) mapping->start);
    // Last page index.
    int j = bytes_to_pages((char*) block_finish - (char*) mapping->start);
    // Update permissions between pages i and f.
    memset(mapping->permissions + i, block_perm, j - i);
    // If permission allow writting or disable it, then also update dirty set.
    if (block_perm & PROT_WRITE) {
        memset(mapping->dirty + i, PROT_WRITE, j - i);
    } else if (block_perm == PROT_NONE) {
        memset(mapping->dirty + i, PROT_NONE, j - i);
    }
    dbg("tracking  %16p - %16p (permissions)\n", block_start, block_finish);
}

void mapping_update_size(mapping_t* head, mapping_t* mapping, void* unmapping_start, void* unmapping_finish) {
    void* mapping_finish = ((char*) mapping->start) + mapping->size;
    size_t unmapping_size = (char*) unmapping_finish - (char*) unmapping_start;

    // If we are removing a block from the start.
    if (unmapping_start == mapping->start && unmapping_finish <= mapping_finish) {
        if (unmapping_size == mapping->size) {
            dbg("tracking  %16p - %16p (deleted)\n", mapping->start, mapping_finish);
            list_mappings_delete(head, mapping);
        } else {
            mapping->size -= unmapping_size;
            mapping->start = unmapping_finish;
            // Note: since we re clipping the range, we also need to clip permissions and dirty metadata.
            // Note 2: the best solution would be to realloc the arrays. We are leaking a bit of memory this way.
            int pages_to_shift = unmapping_size / getpagesize();
            mapping->permissions += pages_to_shift;
            mapping->dirty += pages_to_shift;
            mapping->validated += pages_to_shift;
            dbg("tracking  %16p - %16p (clipping beg shifted %d pages)\n", mapping->start, mapping_finish, pages_to_shift);
        }
    }
    // If we are removing a block from the end.
    else if (unmapping_finish == mapping_finish && unmapping_start >= mapping->start) {
        if (unmapping_size == mapping->size) {
            dbg("tracking  %16p - %16p (deleted)\n", mapping->start, mapping_finish);
            list_mappings_delete(head, mapping);
        } else {
            mapping->size -= unmapping_size;
            mapping_finish = ((char*) mapping->start) + mapping->size;
            // Note: since we re clipping the range, we also need to clip permissions and dirty metadata.
            // Note 2: the best solution would be to realloc the arrays. We are leaking a bit of memory this way.
            dbg("tracking  %16p - %16p (clipping end)\n", mapping->start, mapping_finish);
        }
    }
    // Unsupported unmapping range.
    else {
        err("error: unsupported mapping update: len = %lx [%16p to %16p] from [%16p to %16p]\n",
            unmapping_size, unmapping_start, unmapping_finish, mapping->start, mapping_finish);
    }
}