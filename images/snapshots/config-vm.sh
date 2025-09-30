#!/bin/bash

# Inspired by: https://github.com/firecracker-microvm/firecracker/blob/main/docs/snapshotting/network-for-clones.md
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$3" ]; then
    echo "Syntax: ./config-vm.sh <clone id> <vm ip> <vm image name>"
    echo "Clone id should an integer higher than zero which is not being used by another clone vm."
    echo "Please use a free IP 172.18.[0,255].[3,255]. The IP will be used to create a firecracker directory and to route requests to the vm."
    echo "The snapshot vm ip will be used to find the vm directory where the snapshot resides."
    exit 0
fi

# ID of the VM clone. Used to prepare internal ips.
CLONE_ID=$1

# VM ip accessible to the outside (unique).
PUBLIC_VM_IP=$2

# Image to be used (graalvisor, hotspot, hotspot-agent).
IMAGE_NAME=$3

# ID of the vm (based on the public ip).
VM_ID=$(echo $PUBLIC_VM_IP | tr . -)

function gen_hostns_veth_ip {
    # 10.<idx / 30>.<(idx % 30) * 8>.1/24
    byte1=10
    byte2=$(($CLONE_ID / 30))
    byte3=$(($CLONE_ID % 30))
    byte3=$(($byte3 * 8))
    byte4=1
    echo "$byte1.$byte2.$byte3.$byte4"
}

function gen_vmns_veth_ip {
    # 10.<idx / 30>.<(idx % 30) * 8>.2/24
    byte1=10
    byte2=$(($CLONE_ID / 30))
    byte3=$(($CLONE_ID % 30))
    byte3=$(($byte3 * 8))
    byte4=2
    echo "$byte1.$byte2.$byte3.$byte4"
}

function load_new {
    # Kernel image used in the vm.
    sudo cp $DIR/hello-vmlinux.bin $VM_DIR/
    KERNEL=/hello-vmlinux.bin

    # Root filesystem to be used in the VM.
    sudo cp $DIR/$IMAGE_NAME/$IMAGE_NAME.img $VM_DIR/
    ROOTFS=/$IMAGE_NAME.img

    # VM memory and core config (memory in MB and number of vcores).
    VM_MEM=256
    VM_CPU=1

    # Kernel arguments (including network configuration).
    IMAGE_PATH_ON_DISK="/init"
    KERNEL_CONSOLE_ARGS='console=ttyS0'
    args="LD_LIBRARY_PATH=/lib:/lib64:/apps:/usr/local/lib"
    KERNEL_BOOT_ARGS="init=$IMAGE_PATH_ON_DISK quiet rw tsc=reliable ipv6.disable=1 ip=$VM_TAP_IP::$HOST_TAP_IP:$TAP_MASK_LONG::$VM_DEV:none::: nomodule $KERNEL_CONSOLE_ARGS reboot=k panic=1 pci=off $args"

    # Configures kernel its arguments.
    sudo curl --unix-socket $VM_SOCKET \
        -X PUT "http://localhost/boot-source" \
        --data "{
        \"kernel_image_path\": \"${KERNEL}\",
        \"boot_args\": \"${KERNEL_BOOT_ARGS}\"
        }"

    # Configures the rootfs.
    sudo curl --unix-socket $VM_SOCKET -i \
        -X PUT "http://localhost/drives/rootfs" \
        -d "{
        \"drive_id\": \"rootfs\",
        \"path_on_host\": \"${ROOTFS}\",
        \"is_root_device\": true,
        \"is_read_only\": false
        }"

    # Confiures resources.
    sudo curl --unix-socket $VM_SOCKET \
        -X PUT "http://localhost/machine-config" \
        --data "{
        \"vcpu_count\": ${VM_CPU},
        \"mem_size_mib\": ${VM_MEM},
        \"track_dirty_pages\": false
        }"


    # Confiures network.
    sudo curl --unix-socket $VM_SOCKET -i \
        -X PUT 'http://localhost/network-interfaces/eth0' \
        -d "{
        \"iface_id\": \"${VM_DEV}\",
        \"guest_mac\": \"${VM_MAC}\",
        \"host_dev_name\": \"${TAP}\"
        }"

    # Launches vm.
    sudo curl --unix-socket $VM_SOCKET -i \
        -X PUT "http://localhost/actions" \
        -d "{
        \"action_type\": \"InstanceStart\"
        }"
}

# Internal (vm tap ip) and external (host tap ip) ips and masks (same for all clones).
HOST_TAP_IP=192.168.241.1
VM_TAP_IP=192.168.241.2
TAP=vmtap
TAP_MASK_SHORT=29
TAP_MASK_LONG=255.255.255.248

# Veth ips, mask, and names both in the host and in the vm namespaces.
HOST_NS_VETH_IP=$(gen_hostns_veth_ip)
HOST_NS_VETH=veth$HOST_NS_VETH_IP
VM_NS_VETH_IP=$(gen_vmns_veth_ip)
VM_NS_VETH=veth$VM_NS_VETH_IP
VETH_MASK_SHORT=24

# Default network device and mac used in the vm.
VM_DEV=eth0
VM_MAC=$(printf 'DE:AD:BE:EF:%02X:%02X\n' $((RANDOM%256)) $((RANDOM%256)))

# Create vm tap in vm namespace.
sudo ip netns exec ns$CLONE_ID ip tuntap add dev $TAP mode tap
sudo ip netns exec ns$CLONE_ID ip addr add $HOST_TAP_IP/$TAP_MASK_SHORT dev $TAP
sudo ip netns exec ns$CLONE_ID ip link set dev $TAP up

# Create vm veth pair.
sudo ip netns exec ns$CLONE_ID ip link add $HOST_NS_VETH type veth peer name $VM_NS_VETH
sudo ip netns exec ns$CLONE_ID ip addr add $VM_NS_VETH_IP/$VETH_MASK_SHORT dev $VM_NS_VETH
sudo ip netns exec ns$CLONE_ID ip link set dev $VM_NS_VETH up

# Move one end to the host namespace.
sudo ip netns exec ns$CLONE_ID ip link set $HOST_NS_VETH netns 1
sudo ip addr add $HOST_NS_VETH_IP/$VETH_MASK_SHORT dev $HOST_NS_VETH
sudo ip link set dev $HOST_NS_VETH up

# Designate the outer end as default gateway for packets leaving the namespace.
sudo ip netns exec ns$CLONE_ID ip route add default via $HOST_NS_VETH_IP dev $VM_NS_VETH

# For packets that leave the namespace and have the source ip address of the
# original guest, rewrite the source address to public clone address.
sudo ip netns exec ns$CLONE_ID iptables -t nat -A POSTROUTING -o $VM_NS_VETH -s $VM_TAP_IP -j SNAT --to $PUBLIC_VM_IP

# do the reverse operation; rewrites the destination address of packets
# heading towards the clone address to vm tap ip.
sudo ip netns exec ns$CLONE_ID iptables -t nat -A PREROUTING -i $VM_NS_VETH -d $PUBLIC_VM_IP -j DNAT --to $VM_TAP_IP

# Adds a route on the host for the clone address.
sudo ip route add $PUBLIC_VM_IP via $VM_NS_VETH_IP

# Directory where the socket and logs of the vm will be placed (the link is created to avoid long paths).
VM_DIR=$DIR/$IMAGE_NAME/$PUBLIC_VM_IP/root
rm $VM_DIR &> /dev/null
sudo ln -s $DIR/$IMAGE_NAME/$PUBLIC_VM_IP/firecracker/$VM_ID/root $VM_DIR

# TODO: this is a dirty hack to avoid having socket path too long.
if [ ! -L /tmp/snapshots ]; then
    ln -s $DIR /tmp/snapshots
fi
# Socket that will be used to control the vm.
VM_SOCKET=/tmp/snapshots/$IMAGE_NAME/$PUBLIC_VM_IP/root/firecracker.socket

load_new
