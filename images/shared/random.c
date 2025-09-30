#include <errno.h>
#include <fcntl.h>
#include <linux/random.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

// Note this is obiously not a good practise. Ideally this long input would be passed from a real random source from the outside.
const char random_seed[] = "ed861a41069505917dfa17601b53c750cecf692ddd6669e88c5b56750f92188107edbfbfd9138a70c7dbe9d83d5648020816f64d799cb63f65b975792e3adc5260fdfc23401e5dd6ad1b32deceb83496";

void exit_perror(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Note: based on https://github.com/firecracker-microvm/firecracker/blob/main/docs/snapshotting/random-for-clones.md
void init_entropy_pool() {
    size_t len = 0;
    struct rand_pool_info* info = NULL;

    len = strlen(random_seed);
    // We want len to be a multiple of 8 such that we have an easier time
    // parsing data into an array of u32s.
    if (len % 8) {
        exit_perror("Length of the random seed should a multiple of 8.");
    }

    info = malloc(sizeof(struct rand_pool_info) + len / 8);
    if (info == NULL) {
        exit_perror("Could not alloc rand_pool_info struct");
    }
    // This is measured in bits IIRC.
    info->entropy_count = len * 4;
    info->buf_size = len / 8;

    int fd = open("/dev/urandom", O_RDWR);
    if (fd < 0) {
        exit_perror("Unable to open /dev/urandom");
    }

    if (ioctl(fd, RNDCLEARPOOL) < 0) {
        exit_perror("Error issuing RNDCLEARPOOL operation");
    }

    // Add the entropy bytes supplied by the user.
    char num_buf[9] = {};
    size_t pos = 0;

    while (pos < len) {
        memcpy(num_buf, random_seed + pos, 8);
        info->buf[pos / 8] = strtoul(num_buf, NULL, 16);
        pos += 8;
    }

    if (ioctl(fd, RNDADDENTROPY, info) < 0) {
        exit_perror("Error issuing RNDADDENTROPY operation");
    }
}
