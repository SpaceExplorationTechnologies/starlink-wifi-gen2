#!/bin/sh

if [ ! -n "$2" ]; then
  echo "insufficient arguments!"
  echo "Usage: $0 <lan_if> <wan_if> "
  echo "lan_if: LAN interface name - eth0 or eth2"
  echo "wan_if: WAN interface name - eth1 or eth3"
  echo "example:"
  echo "        $0 eth0 eth1"
  exit 0
fi

#==============================================
#configure Interface
#==============================================
LAN_IF="$1"
WAN_IF="$2"
ALG="aes"
LIVE_TIME="86400"

fw3 stop
#mount_root
#mount_root done
#kmodloader
route del default dev br-lan
route del 255.255.255.255

mkdir /var/log
mkdir /var/run
mkdir /var/run/pluto
#brctl addbr br-lan
#brctl delif br-lan $LAN_IF
#ifconfig $WAN_IF down
#ifconfig $LAN_IF down
#brctl addif br-lan $LAN_IF
#ifconfig $WAN_IF up
#ifconfig $LAN_IF up

#ifconfig br-lan 10.10.10.254 netmask 255.255.255.0
#ifconfig $WAN_IF 10.10.20.254 netmask 255.255.255.0
route add default gw 10.10.20.253


iptables -I FORWARD -s 192.168.1.0/24 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1452
iptables -I FORWARD -d 192.168.2.0/24 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1452
iptables -I FORWARD -s 192.168.1.0/24 -p tcp --tcp-flags SYN,RST SYN  -j TCPMSS --clamp-mss-to-pmtu
iptables -I FORWARD -d 192.168.2.0/24 -p tcp --tcp-flags SYN,RST SYN  -j TCPMSS --clamp-mss-to-pmtu
iptables -t nat -F
iptables -t nat -I POSTROUTING -o $WAN_IF -s 192.168.1.0/24 -j ACCEPT
ifconfig br-lan mtu 1452
echo 1 >/proc/sys/net/ipv4/ip_forward

telnetd
#==============================================
#prepare /etc/ipsec.conf & /etc/ipsec.secrets
#==============================================
echo "
version 2.0

config setup
  interfaces=%defaultroute
  klipsdebug=none
  plutodebug=none
  nat_traversal=no
  protostack=netkey
  plutostderrlog=/var/log/pluto.log

conn CONN2
  left=10.10.20.254
  leftsubnet=192.168.1.0/24
  leftid=@RIGHT
  type=tunnel
  rekey=yes
  rekeymargin=30s
  forceencaps=no
  right=10.10.20.253
  rightsubnet=192.168.2.0/24
  rightid=@LEFT
  auto=add
  auth=esp
  esp=aes128-sha1
  lifetime=86400s
  authby=secret
  ike=aes128-sha1-modp1024
  ikelifetime=86400s
  pfs=no" > /etc/ipsec.conf

echo "@RIGHT @LEFT : PSK \"123456789\"" > /etc/ipsec.secrets


#======================
#establish ipsec tunnel
#======================
IPSEC_CONF_FILE=/etc/ipsec.conf
PLUTO_PID=/var/run/pluto/pluto.pid

KEY_MODE="ike"
CONN_NAME="CONN2"

#If pluto is not running, just start it
if [ ! -f $PLUTO_PID ]; then
                mkdir /etc/ipsec.d/
                mkdir /etc/ipsec.d/cacerts # X.509 root certificates
                mkdir /etc/ipsec.d/certs # X.509 client certificates
                mkdir /etc/ipsec.d/private # X.509 private certificates
                mkdir /etc/ipsec.d/crls # X.509 certificate Revocation lists
                mkdir /etc/ipsec.d/ocspcerts # X.509 online certificate status protocol certificates
                mkdir /etc/ipsec.d/passwd #XAUTH password file
                mkdir /etc/ipsec.d/policies #the opportunistic encryption policy groups

        ipsec setup start
fi

if [ $KEY_MODE = "ike" ]; then
        ipsec auto --replace $CONN_NAME
#       ipsec auto --up $CONN_NAME
else #Manual
        ipsec manual --up $CONN_NAME
fi
