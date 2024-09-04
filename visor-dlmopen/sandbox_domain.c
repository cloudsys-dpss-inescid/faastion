#define _GNU_SOURCE

#include "pkru_sandbox.h"
#include "list.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/syscall.h>

struct domain {
    pthread_mutex_t mutex;
	void *arena;
	int pkey; // FIXME: redundant
	List *threads;
};

static int PAGE_SIZE;
struct domain *domains[DOMAINS];

void *get_arena(int domain) {
    return domains[domain]->arena;
}

int domain_protect(int pkey) {
    struct domain *domain = domains[pkey];
    
    int ret;
    if((ret = syscall(SYS_pkey_alloc, 0, 0)) < 0) {
        fprintf(stderr, "error: failed to allocate pkey %d\n", pkey);
        return -1;
    }
    domain->pkey = pkey;

    // FIXME: does pkey_alloc guarantee sequential key values?
    assert(ret == pkey && "key and domain have different values");

    if (pkey_mprotect(domain->arena, PAGE_SIZE, PROT_READ|PROT_WRITE, pkey)) {
        perror("pkey_mprotect");
        fprintf(stderr, "error: failed protect arena at %p with pkey %d\n", domain->arena, pkey);
        return -1;
	}
    fprintf(stdout, "Protected arena at %p - %p with pkey %d\n", domain->arena, ((char*)domain->arena + PAGE_SIZE), pkey);

    return 0;
}

int domain_init(int pkey) {
    struct domain *domain = (struct domain *)malloc(sizeof(struct domain));
    domains[pkey] = domain; 

    domain->threads = new_list();
    pthread_mutex_init(&domain->mutex, NULL);

    // allocate arena
    void *addr;
    if ((addr = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0)) == MAP_FAILED) {
        fprintf(stderr, "error: failed to mmap arena\n");
        return -1;
    }
    memset(addr, 0, PAGE_SIZE);
    domain->arena = addr;

    return 0;
}

int initialize_domains() {
    PAGE_SIZE = getpagesize();
    domain_init(0);
    for (int i = 1; i < DOMAINS; i++) {
        if (domain_init(i) || domain_protect(i))
            return -1;
    }
    return 0;
}

// TODO - make it thread safe.
int get_thread_domain(pid_t tid)
{
    for (int i = 0; i < DOMAINS; i++) {
        pthread_mutex_lock(&domains[i]->mutex);
        if (lookup_node(domains[i]->threads, tid)) {
            pthread_mutex_unlock(&domains[i]->mutex);
            return i;    
        }
        pthread_mutex_unlock(&domains[i]->mutex);
    }
    return 0;
}

void set_thread_domain(pid_t tid, int domain)
{
    pthread_mutex_lock(&domains[domain]->mutex);
    add_node(domains[domain]->threads, tid);
    pthread_mutex_unlock(&domains[domain]->mutex);
}

void del_thread_domain(pid_t tid, int domain)
{
    pthread_mutex_lock(&domains[domain]->mutex);
    remove_node(domains[domain]->threads, tid);
    pthread_mutex_unlock(&domains[domain]->mutex);
}

// TODO - make it thread safe.
int book_available_domain(pid_t tid)
{
    for (int i = 2; i < DOMAINS; i++) {
        if (get_size(domains[i]->threads) == 0) {
            set_thread_domain(tid, i);
            fprintf(stdout, "Booked domain %d for thread %d\n", i, tid);
            return i;
        }
    }
    return 0;
}

