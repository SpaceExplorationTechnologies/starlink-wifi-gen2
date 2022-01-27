#!/bin/sh

ipt() {
    iptables  $@
    ip6tables $@
}

# Flush any existing rules
ipt -F INPUT
ipt -F OUTPUT
ipt -F FORWARD

# Drop all incoming connections to host by default
ipt -P INPUT DROP
# Drop traffic to LAN clients by default
ipt -P FORWARD DROP

# If bypass mode is disabled, add firewall rules
if [ ! -f "/tmp/enable_bypass_mode" ]; then
    # Allow all outbound traffic
    ipt -P OUTPUT ACCEPT

    # Add INPUT table rules
    # Allow incoming ping requests
    iptables -A INPUT -i br-lan -p icmp --icmp-type echo-request -j ACCEPT
    ip6tables -A INPUT -i br-lan -p icmpv6 --icmpv6-type echo-request -j ACCEPT
    iptables -A INPUT -i br-lan1 -p icmp --icmp-type echo-request -j ACCEPT
    ip6tables -A INPUT -i br-lan1 -p icmpv6 --icmpv6-type echo-request -j ACCEPT
    iptables -A INPUT -i br-lan2 -p icmp --icmp-type echo-request -j ACCEPT
    ip6tables -A INPUT -i br-lan2 -p icmpv6 --icmpv6-type echo-request -j ACCEPT
    iptables -A INPUT -i br-hiddenlan -p icmp --icmp-type echo-request -j ACCEPT
    ip6tables -A INPUT -i br-hiddenlan -p icmpv6 --icmpv6-type echo-request -j ACCEPT
    # Allow incoming ping replies
    iptables -A INPUT -i br-lan -p icmp --icmp-type echo-reply -j ACCEPT
    ip6tables -A INPUT -i br-lan -p icmpv6 --icmpv6-type echo-reply -j ACCEPT
    iptables -A INPUT -i br-lan1 -p icmp --icmp-type echo-reply -j ACCEPT
    ip6tables -A INPUT -i br-lan1 -p icmpv6 --icmpv6-type echo-reply -j ACCEPT
    iptables -A INPUT -i br-lan2 -p icmp --icmp-type echo-reply -j ACCEPT
    ip6tables -A INPUT -i br-lan2 -p icmpv6 --icmpv6-type echo-reply -j ACCEPT
    iptables -A INPUT -i br-hiddenlan -p icmp --icmp-type echo-reply -j ACCEPT
    ip6tables -A INPUT -i br-hiddenlan -p icmpv6 --icmpv6-type echo-reply -j ACCEPT
    # Allow incoming http over bridges (for captive and config portals)
    ipt -A INPUT -i br-lan -p tcp --dport 80 -j ACCEPT
    ipt -A INPUT -i br-lan1 -p tcp --dport 80 -j ACCEPT
    ipt -A INPUT -i br-lan2 -p tcp --dport 80 -j ACCEPT
    # Allow incoming ssh over bridges
    ipt -A INPUT -i br-lan -p tcp --dport 22 -j ACCEPT
    ipt -A INPUT -i br-lan1 -p tcp --dport 22 -j ACCEPT
    ipt -A INPUT -i br-lan2 -p tcp --dport 22 -j ACCEPT
    # Allow incoming DNS requests over bridges
    ipt -A INPUT -i br-lan -p tcp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-lan -p udp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-lan1 -p tcp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-lan1 -p udp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-lan2 -p tcp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-lan2 -p udp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-hiddenlan -p tcp --dport 53 -j ACCEPT
    ipt -A INPUT -i br-hiddenlan -p udp --dport 53 -j ACCEPT
    # Allow incoming DHCP requests over bridges
    ipt -A INPUT -i br-lan -p udp --dport 67 -j ACCEPT
    ipt -A INPUT -i br-lan1 -p udp --dport 67 -j ACCEPT
    ipt -A INPUT -i br-lan2 -p udp --dport 67 -j ACCEPT
    ipt -A INPUT -i br-hiddenlan -p udp --dport 67 -j ACCEPT
    # Allow incoming gRPC commands over bridges
    ipt -A INPUT -i br-lan -p tcp --dport 9000:9002 -j ACCEPT
    ipt -A INPUT -i br-lan1 -p tcp --dport 9000:9002 -j ACCEPT
    ipt -A INPUT -i br-lan2 -p tcp --dport 9000:9002 -j ACCEPT
    ipt -A INPUT -i br-hiddenlan -p tcp --dport 9002 -j ACCEPT
    # Allow any incoming connections from localhost
    ipt -A INPUT -i lo -j ACCEPT
    # Allow incoming established or related connections from br-lan
    ipt -A INPUT -i br-lan -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
    ipt -A INPUT -i br-lan1 -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
    ipt -A INPUT -i br-lan2 -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
    # Allow incoming established or related connections from eth1 (WAN)
    ipt -A INPUT -i eth1 -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

    # Add FORWARD table rules
    # Block traffic between hiddenlan and WAN
    ipt -A FORWARD -i br-hiddenlan -o eth1 -j DROP
    ipt -A FORWARD -i eth1 -o br-hiddenlan -j DROP
    # Allow traffic to WAN from other bridges
    ipt -A FORWARD -o eth1 -j ACCEPT
    # Allow traffic from localhost
    ipt -A FORWARD -i lo -j ACCEPT
    # Allow established or related connections from eth1
    ipt -A FORWARD -i eth1 -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

    # Add NAT rules. Ipv4 only.
    iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
else
    # If bypass mode is enabled, drop all outbound traffic
    ipt -P OUTPUT DROP
fi
