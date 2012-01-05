.PHONY: all all-before all-after clean clean-custom

all: all-before lib CLI ROUTE BRIDGE STP PORT GDD  RSTP VLAN DHCP IPV4 VRRP UDP RTM LIB PLATFORM SWITCH all-after

include make.h

lib:
	mkdir -p obj
	mkdir -p $(PLATFORM_EXE)_obj/
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
	make -C $(CODE_DIR)/vrrp

UDP:
	make -C $(CODE_DIR)/udp
ROUTE:
	make -C $(CODE_DIR)/route
RTM:
#	make -C $(CODE_DIR)/rtm



SWITCH:  lib BRIDGE STP PLATFORM CLI PORT GDD  RSTP VLAN IPV4 VRRP UDP ROUTE
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
