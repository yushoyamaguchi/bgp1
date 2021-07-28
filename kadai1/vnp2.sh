#!/bin/sh

ip netns add c1;
ip netns add c2;
ip netns add c3;
ip netns add c4;
ip netns add r1;
ip netns add r2;
ip netns add r3;

ip link add name r1_net0 type veth peer name r2_net0;
ip link add name c1_net0 type veth peer name r1_net1;
ip link add name c3_net0 type veth peer name r2_net1;
ip link add name c2_net0 type veth peer name r1_net2;
ip link add name c4_net0 type veth peer name r2_net2;
ip link add name r1_net3 type veth peer name r3_net0;
ip link add name r2_net3 type veth peer name r3_net1;

ip link set r1_net0 netns r1;
ip link set r1_net1 netns r1;
ip link set r1_net2 netns r1;
ip link set r2_net0 netns r2;
ip link set r2_net1 netns r2;
ip link set r2_net2 netns r2;
ip link set c1_net0 netns c1;
ip link set c2_net0 netns c2;
ip link set c3_net0 netns c3;
ip link set c4_net0 netns c4;
ip link set r3_net0 netns r3;
ip link set r3_net1 netns r3;
ip link set r1_net3 netns r1;
ip link set r2_net3 netns r2;

ip netns exec c1 ip addr add 10.1.0.2/24 dev c1_net0;
ip netns exec c2 ip addr add 10.2.0.2/24 dev c2_net0;
ip netns exec r1 ip addr add 10.2.0.1/24 dev r1_net2;
ip netns exec r1 ip addr add 10.1.0.1/24 dev r1_net1;
ip netns exec r1 ip addr add 10.255.1.1/24 dev r1_net0;
ip netns exec r2 ip addr add 10.255.1.2/24 dev r2_net0;
ip netns exec r2 ip addr add 10.3.0.1/24 dev r2_net1;
ip netns exec r2 ip addr add 10.4.0.1/24 dev r2_net2;
ip netns exec c3 ip addr add 10.3.0.2/24 dev c3_net0;
ip netns exec c4 ip addr add 10.4.0.2/24 dev c4_net0;
ip netns exec r1 ip addr add 10.255.2.1/24 dev r1_net3;
ip netns exec r2 ip addr add 10.255.3.1/24 dev r2_net3;
ip netns exec r3 ip addr add 10.255.2.2/24 dev r3_net0;
ip netns exec r3 ip addr add 10.255.3.2/24 dev r3_net1;

ip netns exec r1 ip link set r1_net0 up;
ip netns exec r1 ip link set r1_net1 up;
ip netns exec r1 ip link set r1_net2 up;
ip netns exec r2 ip link set r2_net0 up;
ip netns exec r2 ip link set r2_net1 up;
ip netns exec r2 ip link set r2_net2 up;
ip netns exec c1 ip link set c1_net0 up;
ip netns exec c2 ip link set c2_net0 up;
ip netns exec c3 ip link set c3_net0 up;
ip netns exec c4 ip link set c4_net0 up;
ip netns exec r1 ip link set lo up;
ip netns exec r2 ip link set lo up;
ip netns exec c1 ip link set lo up;
ip netns exec c2 ip link set lo up;
ip netns exec c3 ip link set lo up;
ip netns exec c4 ip link set lo up;
ip netns exec r3 ip link set r3_net0 up;
ip netns exec r3 ip link set r3_net1 up;
ip netns exec r1 ip link set r1_net3 up;
ip netns exec r2 ip link set r2_net3 up;
ip netns exec r3 ip link set lo up;

ip netns exec r1 sysctl -w net.ipv4.ip_forward=1;
ip netns exec r2 sysctl -w net.ipv4.ip_forward=1;
ip netns exec r3 sysctl -w net.ipv4.ip_forward=1;

ip netns exec c1 ip route add 0.0.0.0/0 via 10.1.0.1;
ip netns exec c3 ip route add 0.0.0.0/0 via 10.3.0.1;
ip netns exec c2 ip route add 0.0.0.0/0 via 10.2.0.1;
ip netns exec c4 ip route add 0.0.0.0/0 via 10.4.0.1;
ip netns exec r1 ip route add 0.0.0.0/0 via 10.255.1.2 dev r1_net0;
ip netns exec r2 ip route add 0.0.0.0/0 via 10.255.1.1 dev r2_net0;
ip netns exec r3 ip route add 10.3.0.0/24 via 10.255.3.1 dev r3_net1;
ip netns exec r3 ip route add 10.1.0.0/24 via 10.255.2.1 dev r3_net0;
ip netns exec r3 ip route add 10.4.0.0/24 via 10.255.3.1 dev r3_net1;
ip netns exec r3 ip route add 10.2.0.0/24 via 10.255.2.1 dev r3_net0;
ip netns exec r1 ip route add 10.255.3.0/24 via 10.255.2.2 dev r1_net3;
ip netns exec r2 ip route add 10.255.2.0/24 via 10.255.3.2 dev r2_net3;

