#!/bin/bash

echo 4194304 > /proc/sys/kernel/threads-max
echo 4194304 > /proc/sys/kernel/pid_max
echo 4194304 > /proc/sys/vm/max_map_count

# Debian 11
echo 4194304 > /sys/fs/cgroup/user.slice/user-1000.slice/pids.max

# Firecracker ubuntu bionic.
echo 4194304 > '/sys/fs/cgroup/pids/system.slice/system-serial\x2dgetty.slice/serial-getty@ttyS0.service'/pids.max
