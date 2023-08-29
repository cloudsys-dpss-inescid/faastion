#include <stdio.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <sys/resource.h>
#include "hello.skel.h"

static void bump_memlock_rlimit() {
	struct rlimit rlim_new = {
		.rlim_cur = RLIM_INFINITY,
		.rlim_max = RLIM_INFINITY,
	};

	if (setrlimit(RLIMIT_MEMLOCK, &rlim_new)) {
		perror("setrlimit");
		exit(1);
	}
}

int main() {
	bump_memlock_rlimit();

	struct hello *skel = hello__open();
	hello__load(skel);
	hello__attach(skel);

	pid_t key = getpid();
	uint64_t value = 0;
	if (bpf_map__update_elem(skel->maps.pid_map, &key, sizeof(key), &value, sizeof(value), 0)) {
		fprintf(stderr, "Failed to update map element\n");
		exit(1);
	}

	printf("Hello World!\n");

	hello__destroy(skel);

	return 0;
}
