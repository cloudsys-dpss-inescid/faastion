#!/bin/bash

# This namespace creation sequence is based on what is used to create firecracker VMs from snapshots (see demos/firecracker-netns).

# ID of the VM clone. Used to prepare internal ips.
CLONE_ID=$1

# VM ip accessible to the outside (unique).
PUBLIC_VM_IP=123.123.123.$CLONE_ID

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

# Create namespace
start=$(date +%s%N)
sudo ip netns add ns$CLONE_ID
end=$(date +%s%N)
echo "Netns creation took $((($end - $start)/1000)) us"

# Create vm tap in vm namespace.
start=$(date +%s%N)
sudo ip netns exec ns$CLONE_ID ip tuntap add dev $TAP mode tap
sudo ip netns exec ns$CLONE_ID ip addr add $HOST_TAP_IP/$TAP_MASK_SHORT dev $TAP
sudo ip netns exec ns$CLONE_ID ip link set dev $TAP up
end=$(date +%s%N)
echo "Tap creation took $((($end - $start)/1000)) us"

# Create vm veth pair.
start=$(date +%s%N)
sudo ip netns exec ns$CLONE_ID ip link add $HOST_NS_VETH type veth peer name $VM_NS_VETH
sudo ip netns exec ns$CLONE_ID ip addr add $VM_NS_VETH_IP/$VETH_MASK_SHORT dev $VM_NS_VETH
sudo ip netns exec ns$CLONE_ID ip link set dev $VM_NS_VETH up
end=$(date +%s%N)
echo "Veth creation took $((($end - $start)/1000)) us"

# Move one end to the host namespace.
start=$(date +%s%N)
sudo ip netns exec ns$CLONE_ID ip link set $HOST_NS_VETH netns 1
sudo ip addr add $HOST_NS_VETH_IP/$VETH_MASK_SHORT dev $HOST_NS_VETH
sudo ip link set dev $HOST_NS_VETH up
end=$(date +%s%N)
echo "Veth setup took $((($end - $start)/1000)) us"

# Designate the outer end as default gateway for packets leaving the namespace.
start=$(date +%s%N)
sudo ip netns exec ns$CLONE_ID ip route add default via $HOST_NS_VETH_IP dev $VM_NS_VETH

# For packets that leave the namespace and have the source ip address of the
# original guest, rewrite the source address to public clone address.
sudo ip netns exec ns$CLONE_ID iptables -t nat -A POSTROUTING -o $VM_NS_VETH -s $VM_TAP_IP -j SNAT --to $PUBLIC_VM_IP

# do the reverse operation; rewrites the destination address of packets
# heading towards the clone address to vm tap ip.
sudo ip netns exec ns$CLONE_ID iptables -t nat -A PREROUTING -i $VM_NS_VETH -d $PUBLIC_VM_IP -j DNAT --to $VM_TAP_IP

# Adds a route on the host for the clone address.
sudo ip route add $PUBLIC_VM_IP via $VM_NS_VETH_IP
end=$(date +%s%N)
echo "Routes setup took $((($end - $start)/1000)) us"

# Workload!
sleep 1

# Delete namespace (and all attached to it).
start=$(date +%s%N)
sudo ip netns delete ns$CLONE_ID
end=$(date +%s%N)
echo "Deletion took $((($end - $start)/1000)) us"
