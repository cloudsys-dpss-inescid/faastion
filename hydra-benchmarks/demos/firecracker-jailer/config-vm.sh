#!/bin/bash

# Inspired by: https://gruchalski.com/posts/2021-02-13-launching-alpine-linux-on-firecracker-like-a-boss/
# Inspired by: https://betterprogramming.pub/getting-started-with-firecracker-a88495d656d9

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$1" ]; then
    echo "Please use a free IP 172.18.[0,255].[2,255]. The IP will be used to create a firecracker directory and tap."
    exit 0
fi

# IP and ID to be used by the VM.
VM_IP=$1
VM_ID=$(echo $VM_IP | tr . -)

# Directory where the socket and logs of the VM will be placed (the link is created to avoid long paths).
ln -s $DIR/$VM_IP/firecracker-v1.1.0-x86_64/$VM_ID/root $DIR/$VM_IP/root
VM_DIR=$DIR/$VM_IP/root

# Socket that will be used to control the VM.
VM_SOCKET=$VM_DIR/firecracker.socket

# Tap that will be created on the host to communicate with the VM.
VM_TAP=tap$VM_IP

# Default network device used in the VM.
VM_DEV=eth0

# MAC address generated for the VM.
VM_MAC=$(printf 'DE:AD:BE:EF:%02X:%02X\n' $((RANDOM%256)) $((RANDOM%256)))

# IP address of the host brige (172.18.0.0/16). Used as gateway for VM taps.
VM_GW=172.18.0.1

# Host default device.
HOST_DEV=$(ip route get 8.8.8.8 | grep -Po '(?<=(dev ))(\S+)')

# Host bridge where the taps are connected.
HOST_BRIDGE=testbridge

# Network mask of the VM network (long version).
VM_MK_LONG=255.255.0.0

# Network mask of the VM network (shot version).
VM_MK_SHORT=16

# Kernel image used in the VM.
cp $DIR/hello-vmlinux.bin $VM_DIR/
KERNEL=/hello-vmlinux.bin

# Kernel arguments (including network configuration).
KERNEL_BOOT_ARGS="ro console=ttyS0 noapic reboot=k panic=1 pci=off nomodules random.trust_cpu=on ip=$VM_IP::$VM_GW:$VM_MK_LONG::$VM_DEV:off"

# Root filesystem to be used in the VM.
cp $DIR/hello-rootfs.ext4 $VM_DIR/
ROOTFS=/hello-rootfs.ext4

# VM memory and core config (memory in MB and number of vcores).
VM_MEM=64
VM_CPU=1

# Create a new tap for the vm.
sudo ip tuntap add dev "$VM_TAP" mode tap

# Creating a bridge if there is not one already.
if [ ! -d "/sys/class/net/$HOST_BRIDGE" ]; then
    sudo ip link add name $HOST_BRIDGE type bridge
    sudo ip addr add $VM_GW/$VM_MK_SHORT brd + dev $HOST_BRIDGE
    sudo ip link set dev $HOST_BRIDGE up
    sudo iptables -A FORWARD -o $HOST_BRIDGE -j ACCEPT
    sudo iptables -A FORWARD -i $HOST_BRIDGE -j ACCEPT
    sudo iptables -t nat -A POSTROUTING -o $HOST_DEV -j MASQUERADE
    sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
fi

# Add tap to bridge.
sudo brctl addif $HOST_BRIDGE $VM_TAP

# Enabling the vm tap.
sudo ip link set dev "$VM_TAP" up

# Configures kernel its arguments.
curl --unix-socket $VM_SOCKET \
    -X PUT "http://localhost/boot-source" \
    --data "{
        \"kernel_image_path\": \"${KERNEL}\",
        \"boot_args\": \"${KERNEL_BOOT_ARGS}\"
    }"

# Configures the rootfs.
curl --unix-socket $VM_SOCKET -i \
    -X PUT "http://localhost/drives/rootfs" \
    -d "{
        \"drive_id\": \"rootfs\",
        \"path_on_host\": \"${ROOTFS}\",
        \"is_root_device\": true,
        \"is_read_only\": false
    }"

# Confiures resources.
curl --unix-socket $VM_SOCKET \
    -X PUT "http://localhost/machine-config" \
    --data "{
        \"vcpu_count\": ${VM_CPU},
        \"mem_size_mib\": ${VM_MEM},
        \"track_dirty_pages\": false
    }"

# Confiures network.
curl --unix-socket $VM_SOCKET -i \
    -X PUT 'http://localhost/network-interfaces/eth0' \
    -d "{
        \"iface_id\": \"${VM_DEV}\",
        \"guest_mac\": \"${VM_MAC}\",
        \"host_dev_name\": \"${VM_TAP}\"
    }"

# Launches vm.
curl --unix-socket $VM_SOCKET -i \
    -X PUT "http://localhost/actions" \
    -d "{
        \"action_type\": \"InstanceStart\"
    }"
