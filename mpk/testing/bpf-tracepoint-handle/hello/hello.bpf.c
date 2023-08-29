#include <stdint.h>
#include <sys/types.h>
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

struct write_params_t {
	uint64_t unused1;
	uint64_t unused2;
	uint64_t unused3;
	const char *buf;
};

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 8192);
	__type(key, pid_t);
	__type(value, uint64_t);
} pid_map SEC(".maps");

SEC("tracepoint/syscalls/sys_enter_write")
int bpf_enter_write(struct write_params_t *params) {
	pid_t pid = bpf_get_current_pid_tgid();
	void *elem;

	if ((elem = bpf_map_lookup_elem(&pid_map, &pid)) == NULL) {
		return 0;
	}

	uint64_t value = *(uint64_t*)elem;
	if (value == 0) {
		bpf_printk("%s\n", params->buf);
	}

	return 0;
}

char LICENSE[] SEC("license") = "GPL";
