.PHONY: all all-before all-after clean clean-custom

all: all-before lib INIT CLI ROUTE BRIDGE STP PORT GDD  RSTP VLAN DHCP DHCPSRV DNS ARP IPV4 TELNETD VRRP TCP UDP SOCK SNMP SNTP RTM BGP UTIL LIB PLATFORM SWITCH all-after

include make.h

lib:
	mkdir -p obj
	mkdir -p $(PLATFORM_EXE)_obj/
INIT:
	make -C $(CODE_DIR)/init
BRIDGE:
	make -C $(CODE_DIR)/bridge
STP:
	make -C $(CODE_DIR)/bridge/stp
PLATFORM:
	make -C $(CODE_DIR)/platform
CLI:
	make -C $(CODE_DIR)/cli
PORT:
	make -C $(CODE_DIR)/ifmgmt
GDD:
	make -C $(CODE_DIR)/gdd
VLAN:
	make -C $(CODE_DIR)/vlan

RSTP:
	make -C $(CODE_DIR)/bridge/rstp
IPV4:
	make -C $(CODE_DIR)/ipv4

DHCP:
	make -C $(CODE_DIR)/dhcp

LIB:
	make -C $(CODE_DIR)/lib
VRRP:
	#make -C $(CODE_DIR)/vrrp

UDP:
	make -C $(CODE_DIR)/udp
ROUTE:
	make -C $(CODE_DIR)/route
RTM:
	make -C $(CODE_DIR)/rtm
TCP:
	make -C $(CODE_DIR)/tcp
ARP:
	make -C $(CODE_DIR)/arp
DNS:
	make -C $(CODE_DIR)/dns
SNMP:
	make -C $(CODE_DIR)/snmp
SNTP:
	make -C $(CODE_DIR)/sntp
BGP:
#	make -C $(CODE_DIR)/bgp
DHCPSRV:
	#make -C $(CODE_DIR)/dhcpserver
SOCK:
	make -C $(CODE_DIR)/sockets
TELNETD:
	make -C $(CODE_DIR)/telnetd
UTIL:
	make -C $(CODE_DIR)/utils


SWITCH:  lib INIT CLI ROUTE BRIDGE STP PORT GDD  RSTP VLAN DHCP DHCPSRV DNS ARP IPV4 TELNETD VRRP TCP UDP RTM BGP SNMP UTIL SNTP LIB PLATFORM
	gcc -o Switch.exe obj/*.o $(PLATFORM_EXE)_obj/*.o -lpthread -lrt

clean: clean-custom
	$(RM) obj/*.o
	$(RM) -rf $(PLATFORM_EXE)_obj/  obj/
	make clean -C $(CODE_DIR)/bridge
	make clean -C $(CODE_DIR)/bridge/stp
	make clean -C $(CODE_DIR)/cli
	make clean -C $(CODE_DIR)/ifmgmt
	make clean -C $(CODE_DIR)/gdd
	make clean -C $(CODE_DIR)/bridge/rstp
	make clean -C $(CODE_DIR)/vlan
	make clean -C $(CODE_DIR)/lib
	make clean -C $(CODE_DIR)/dhcp
	make clean -C $(CODE_DIR)/ipv4
	make clean -C $(CODE_DIR)/platform
	make clean -C $(CODE_DIR)/vrrp
	make clean -C $(CODE_DIR)/udp
	make clean -C $(CODE_DIR)/route
	make clean -C $(CODE_DIR)/rtm
	make clean -C $(CODE_DIR)/init
	make clean -C $(CODE_DIR)/tcp
	make clean -C $(CODE_DIR)/arp
	make clean -C $(CODE_DIR)/dns
	make clean -C $(CODE_DIR)/snmp
	make clean -C $(CODE_DIR)/bgp
	make clean -C $(CODE_DIR)/sntp
	make clean -C $(CODE_DIR)/sockets
	make clean -C $(CODE_DIR)/telnetd
	make clean -C $(CODE_DIR)/utils
	#make clean -C $(CODE_DIR)/dhcpserver
