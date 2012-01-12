#ifndef _IFMGMT_H
#define _IFMGMT_H
#include "linklist.h"
struct interface;
#include "netif.h"
#define ZEBRA_INTERFACE_ACTIVE     (1 << 0)
#define ZEBRA_INTERFACE_SUB        (1 << 1)
#define ZEBRA_INTERFACE_LINKDETECTION (1 << 2)

struct interface {
        int32_t   ifIndex;
	void      *platform;
        char      ifDescr[MAX_PORT_NAME];          
        int32_t   ifType;
        int32_t   ifMtu;   
        int32_t   ifSpeed;
        int32_t   ifAdminStatus;
        int32_t   ifOperStatus;
        uint32_t  ifLastChange; 
        uint32_t  ifInOctets; 
        uint32_t  ifInUcastPkts;
        uint32_t  ifInDiscards;    
        uint32_t  ifInErrors;     
        uint32_t  ifInUnknownProtos;
        uint32_t  ifOutOctets;
        uint32_t  ifOutUcastPkts;
        uint32_t  ifOutDiscards;   
        uint32_t  ifOutErrors;    
 	uint32_t   metric;
	struct stp_port_entry *pstp_info;
        unsigned long in_bytes;
        unsigned long in_packets;
        unsigned long in_mcast_pkts;
        unsigned long in_errors;
        unsigned long in_discards;
        unsigned long in_unknown_protos;

        unsigned long out_bytes;
        unsigned long out_packets;
        unsigned long out_mcast_pkts; 
        unsigned long out_errors;
        unsigned long out_discards;
        unsigned long out_collisions;

	/** IP address configuration in network byte order */
	ip_addr_t ip_addr;
	ip_addr_t netmask;
	ip_addr_t gw;

	/** This function is called by the network device driver
	 *  to pass a packet up the TCP/IP stack. */
	if_input_fn input;
	/** This function is called by the IP module when it wants
	 *  to send a packet on the interface. This function typically
	 *  first resolves the hardware address, then sends the packet. */
	if_output_fn output;
	/** This function is called by the ARP module when it wants
	 *  to send a packet on the interface. This function outputs
	 *  the pbuf as-is on the link medium. */
	if_linkoutput_fn linkoutput;
#if LWIP_NETIF_STATUS_CALLBACK
	/** This function is called when the netif state is set to up or down
	*/
	if_status_callback_fn status_callback;
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
	/** This function is called when the netif link is set to up or down
	*/
	if_status_callback_fn link_callback;
#endif /* LWIP_NETIF_LINK_CALLBACK */
	/** This field can be set by the device driver and could point
	 *  to state information for the device. */
	void *state;
#if LWIP_DHCP
	/** the DHCP client state information for this netif */
	struct dhcp *dhcp;
#endif /* LWIP_DHCP */
#if LWIP_AUTOIP
	/** the AutoIP client state information for this netif */
	struct autoip *autoip;
#endif
#if LWIP_NETIF_HOSTNAME
	/* the hostname for this netif, NULL is a valid value */
	char*  hostname;
#endif /* LWIP_NETIF_HOSTNAME */
	/** number of bytes used in ifPhysAddress */
	u8_t ifPhysAddress_len;
	/** link level hardware address of this interface */
	u8_t ifPhysAddress[NETIF_MAX_HWADDR_LEN];
	/** flags (see NETIF_FLAG_ above) */
	u8_t flags;
	/** number of this interface */
	u8_t num;
#if LWIP_SNMP
	/** link type (from "snmp_ifType" enum from snmp.h) */
	u8_t link_type;
	/** (estimate) link speed */
	u32_t link_speed;
	/** timestamp at last change made (up/down) */
	u32_t ts;
	/** counters */
	u32_t ifinoctets;
	u32_t ifinucastpkts;
	u32_t ifinnucastpkts;
	u32_t ifindiscards;
	u32_t ifoutoctets;
	u32_t ifoutucastpkts;
	u32_t ifoutnucastpkts;
	u32_t ifoutdiscards;
#endif /* LWIP_SNMP */
#if LWIP_IGMP
	/** This function could be called to add or delete a entry in the multicast
	  filter table of the ethernet MAC.*/
	if_igmp_mac_filter_fn igmp_mac_filter;
#endif /* LWIP_IGMP */
#if LWIP_NETIF_HWADDRHINT
	u8_t *addr_hint;
#endif /* LWIP_NETIF_HWADDRHINT */
#if ENABLE_LOOPBACK
	/* List of packets to be queued for ourselves. */
	struct pbuf *loop_first;
	struct pbuf *loop_last;
#if LWIP_LOOPBACK_MAX_PBUFS
	u16_t loop_cnt_current;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
#endif /* ENABLE_LOOPBACK */
	  /* Connected address list. */
  	struct list *connected;

	void *info;
}__attribute__ ((__packed__));

typedef struct interface if_t;

extern if_t port_cdb[];

#define   IF_INFO(port)             &port_cdb[port - 1]
#define   IF_INDEX(port)            port_cdb[port - 1].ifIndex
#define   IF_DESCR(port)            port_cdb[port - 1].ifDescr
#define   IF_TYPE(port)             port_cdb[port - 1].ifType
#define   IF_MTU(port)              port_cdb[port - 1].ifMtu
#define   IF_IFSPEED(port)          port_cdb[port - 1].ifSpeed
#define   IF_ADMIN_STATUS(port)     port_cdb[port - 1].ifAdminStatus
#define   IF_OPER_STATUS(port)      port_cdb[port - 1].ifOperStatus
#define   IF_LAST_CHGE(port)        port_cdb[port - 1].ifLastChange
#define   IF_IN_OCTS(port)          port_cdb[port - 1].ifInOctets
#define   IF_IN_UCAST_PKTS(port)    port_cdb[port - 1].ifInUcastPkts
#define   IF_IN_DISCARDS(port)      port_cdb[port - 1].ifInDiscards
#define   IF_IN_ERRORS(port)        port_cdb[port - 1].ifInErrors
#define   IF_IN_UNKNOWN_PROTO(port) port_cdb[port - 1].ifInUnknownProtos
#define   IF_OUT_OCTS(port)         port_cdb[port - 1].ifOutOctets
#define   IF_OUT_UCAST_PKTS(port)   port_cdb[port - 1].ifOutUcastPkts
#define   IF_OUT_DISCARDS(port)     port_cdb[port - 1].ifOutDiscards
#define   IF_OUT_ERRORS(port)       port_cdb[port - 1].ifOutErrors
#define   IF_STP_STATE(port)        port_cdb[port - 1].pstp_info->state
#define   IF_STP_INFO(port)         port_cdb[port - 1].pstp_info
#endif
