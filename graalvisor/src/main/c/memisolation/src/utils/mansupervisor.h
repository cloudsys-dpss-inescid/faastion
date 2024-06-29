/*
 * man_supervisor.h
 *
 */

#ifndef MANSUPER_H
#define MANSUPER_H

#include <stdatomic.h>
#include <poll.h>

// Max number of file descriptors the supervisor can poll
#define MAX_FDS 1024 

// Poll information on all fds
extern struct pollfd fds[MAX_FDS];

// Tracks the usage of a file descriptor
extern volatile atomic_int fd_list[MAX_FDS];

/**
 * @brief Adds the file descriptor to a maintained list polled fds 
 * 
 * @param fd file descriptor to poll
 */
void add_fd(int fd);

/**
 * @brief Removes a file descriptor at specified index from a maintained list of polled fd
 * 
 * @param index 
 */
void remove_fd(int index);


/**
 * @brief Obtains the file descriptor for /dev/null
 * 
 * @return int 
 */
int init_null_fd();

#endif