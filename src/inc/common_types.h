#ifndef _COMMON_TYPES_H_
#define _COMMON_TYPES_H_
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <getopt.h>
#include "os_defs.h"

#ifndef STDIN_FILENO
# define STDIN_FILENO    0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO   1
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO   2
#endif

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(__386__)
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#else
# error "Can't determine endianness"
#endif


/* Protocol families.  */
#define PF_UNSPEC       0       /* Unspecified.  */
#define PF_LOCAL        1       /* Local to host (pipes and file-domain).  */
#define PF_UNIX         PF_LOCAL /* POSIX name for PF_LOCAL.  */
#define PF_FILE         PF_LOCAL /* Another non-standard name for PF_LOCAL.  */
#define PF_INET         2       /* IP protocol family.  */
#define PF_AX25         3       /* Amateur Radio AX.25.  */
#define PF_IPX          4       /* Novell Internet Protocol.  */
#define PF_APPLETALK    5       /* Appletalk DDP.  */
#define PF_NETROM       6       /* Amateur radio NetROM.  */
#define PF_BRIDGE       7       /* Multiprotocol bridge.  */
#define PF_ATMPVC       8       /* ATM PVCs.  */
#define PF_X25          9       /* Reserved for X.25 project.  */
#define PF_INET6        10      /* IP version 6.  */
#define PF_ROSE         11      /* Amateur Radio X.25 PLP.  */
#define PF_DECnet       12      /* Reserved for DECnet project.  */
#define PF_NETBEUI      13      /* Reserved for 802.2LLC project.  */
#define PF_SECURITY     14      /* Security callback pseudo AF.  */
#define PF_KEY          15      /* PF_KEY key management API.  */
#define PF_NETLINK      16
#define PF_ROUTE        PF_NETLINK /* Alias to emulate 4.4BSD.  */
#define PF_PACKET       17      /* Packet family.  */
#define PF_ASH          18      /* Ash.  */
#define PF_ECONET       19      /* Acorn Econet.  */
#define PF_ATMSVC       20      /* ATM SVCs.  */
#define PF_RDS          21      /* RDS sockets.  */
#define PF_SNA          22      /* Linux SNA Project */
#define PF_IRDA         23      /* IRDA sockets.  */
#define PF_PPPOX        24      /* PPPoX sockets.  */
#define PF_WANPIPE      25      /* Wanpipe API sockets.  */
#define PF_LLC          26      /* Linux LLC.  */
#define PF_CAN          29      /* Controller Area Network.  */
#define PF_TIPC         30      /* TIPC sockets.  */
#define PF_BLUETOOTH    31      /* Bluetooth sockets.  */
#define PF_IUCV         32      /* IUCV sockets.  */
#define PF_RXRPC        33      /* RxRPC sockets.  */
#define PF_ISDN         34      /* mISDN sockets.  */
#define PF_PHONET       35      /* Phonet sockets.  */
#define PF_IEEE802154   36      /* IEEE 802.15.4 sockets.  */
#define PF_MAX          37      /* For now..  */

/* Address families.  */
#define AF_UNSPEC       PF_UNSPEC
#define AF_LOCAL        PF_LOCAL
#define AF_UNIX         PF_UNIX
#define AF_FILE         PF_FILE
#define AF_INET         PF_INET
#define AF_AX25         PF_AX25
#define AF_IPX          PF_IPX
#define AF_APPLETALK    PF_APPLETALK
#define AF_NETROM       PF_NETROM
#define AF_BRIDGE       PF_BRIDGE
#define AF_ATMPVC       PF_ATMPVC
#define AF_X25          PF_X25
#define AF_INET6        PF_INET6
#define AF_ROSE         PF_ROSE
#define AF_DECnet       PF_DECnet
#define AF_NETBEUI      PF_NETBEUI
#define AF_SECURITY     PF_SECURITY
#define AF_KEY          PF_KEY
#define AF_NETLINK      PF_NETLINK
#define AF_ROUTE        PF_ROUTE
#define AF_PACKET       PF_PACKET
#define AF_ASH          PF_ASH
#define AF_ECONET       PF_ECONET
#define AF_ATMSVC       PF_ATMSVC
#define AF_RDS          PF_RDS
#define AF_SNA          PF_SNA
#define AF_IRDA         PF_IRDA
#define AF_PPPOX        PF_PPPOX
#define AF_WANPIPE      PF_WANPIPE
#define AF_LLC          PF_LLC
#define AF_CAN          PF_CAN
#define AF_TIPC         PF_TIPC
#define AF_BLUETOOTH    PF_BLUETOOTH
#define AF_IUCV         PF_IUCV
#define AF_RXRPC        PF_RXRPC
#define AF_ISDN         PF_ISDN
#define AF_PHONET       PF_PHONET
#define AF_IEEE802154   PF_IEEE802154
#define AF_MAX          PF_MAX

/* Packet types.  */    
                
#define PACKET_HOST             0               /* To us.  */
#define PACKET_BROADCAST        1               /* To all.  */
#define PACKET_MULTICAST        2               /* To group.  */
#define PACKET_OTHERHOST        3               /* To someone else.  */
#define PACKET_OUTGOING         4               /* Originated by us . */
#define PACKET_LOOPBACK         5
#define PACKET_FASTROUTE        6
    

/* Standard well-defined IP protocols.  */
enum
  {
    IPPROTO_IP = 0,	   /* Dummy protocol for TCP.  */
#define IPPROTO_IP		IPPROTO_IP
    IPPROTO_HOPOPTS = 0,   /* IPv6 Hop-by-Hop options.  */
#define IPPROTO_HOPOPTS		IPPROTO_HOPOPTS
    IPPROTO_ICMP = 1,	   /* Internet Control Message Protocol.  */
#define IPPROTO_ICMP		IPPROTO_ICMP
    IPPROTO_IGMP = 2,	   /* Internet Group Management Protocol. */
#define IPPROTO_IGMP		IPPROTO_IGMP
    IPPROTO_IPIP = 4,	   /* IPIP tunnels (older KA9Q tunnels use 94).  */
#define IPPROTO_IPIP		IPPROTO_IPIP
    IPPROTO_TCP = 6,	   /* Transmission Control Protocol.  */
#define IPPROTO_TCP		IPPROTO_TCP
    IPPROTO_EGP = 8,	   /* Exterior Gateway Protocol.  */
#define IPPROTO_EGP		IPPROTO_EGP
    IPPROTO_PUP = 12,	   /* PUP protocol.  */
#define IPPROTO_PUP		IPPROTO_PUP
    IPPROTO_UDP = 17,	   /* User Datagram Protocol.  */
#define IPPROTO_UDP		IPPROTO_UDP
    IPPROTO_IDP = 22,	   /* XNS IDP protocol.  */
#define IPPROTO_IDP		IPPROTO_IDP
    IPPROTO_TP = 29,	   /* SO Transport Protocol Class 4.  */
#define IPPROTO_TP		IPPROTO_TP
    IPPROTO_DCCP = 33,	   /* Datagram Congestion Control Protocol.  */
#define IPPROTO_DCCP		IPPROTO_DCCP
    IPPROTO_IPV6 = 41,     /* IPv6 header.  */
#define IPPROTO_IPV6		IPPROTO_IPV6
    IPPROTO_ROUTING = 43,  /* IPv6 routing header.  */
#define IPPROTO_ROUTING		IPPROTO_ROUTING
    IPPROTO_FRAGMENT = 44, /* IPv6 fragmentation header.  */
#define IPPROTO_FRAGMENT	IPPROTO_FRAGMENT
    IPPROTO_RSVP = 46,	   /* Reservation Protocol.  */
#define IPPROTO_RSVP		IPPROTO_RSVP
    IPPROTO_GRE = 47,	   /* General Routing Encapsulation.  */
#define IPPROTO_GRE		IPPROTO_GRE
    IPPROTO_ESP = 50,      /* encapsulating security payload.  */
#define IPPROTO_ESP		IPPROTO_ESP
    IPPROTO_AH = 51,       /* authentication header.  */
#define IPPROTO_AH		IPPROTO_AH
    IPPROTO_ICMPV6 = 58,   /* ICMPv6.  */
#define IPPROTO_ICMPV6		IPPROTO_ICMPV6
    IPPROTO_NONE = 59,     /* IPv6 no next header.  */
#define IPPROTO_NONE		IPPROTO_NONE
    IPPROTO_DSTOPTS = 60,  /* IPv6 destination options.  */
#define IPPROTO_DSTOPTS		IPPROTO_DSTOPTS
    IPPROTO_MTP = 92,	   /* Multicast Transport Protocol.  */
#define IPPROTO_MTP		IPPROTO_MTP
    IPPROTO_ENCAP = 98,	   /* Encapsulation Header.  */
#define IPPROTO_ENCAP		IPPROTO_ENCAP
    IPPROTO_PIM = 103,	   /* Protocol Independent Multicast.  */
#define IPPROTO_PIM		IPPROTO_PIM
    IPPROTO_COMP = 108,	   /* Compression Header Protocol.  */
#define IPPROTO_COMP		IPPROTO_COMP
    IPPROTO_SCTP = 132,	   /* Stream Control Transmission Protocol.  */
#define IPPROTO_SCTP		IPPROTO_SCTP
    IPPROTO_UDPLITE = 136, /* UDP-Lite protocol.  */
#define IPPROTO_UDPLITE		IPPROTO_UDPLITE
    IPPROTO_RAW = 255,	   /* Raw IP packets.  */
#define IPPROTO_RAW		IPPROTO_RAW
    IPPROTO_MAX
  };


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t u8_t;
typedef int8_t s8_t;
typedef uint16_t u16_t;
typedef int16_t  s16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;
typedef unsigned long  mem_ptr_t;





typedef uint32_t in_addr_t;

struct in_addr {
  in_addr_t s_addr;
};

typedef struct ip_addr ip_addr_t;

struct in6_addr {
	uint8_t  s6_addr[16];  /* IPv6 address */
};


# define SIN6_LEN


struct sockaddr_in6 
{ 
	uint8_t sin6_len; /* length of this structure */ 
	uint8_t sin6_family; /* AF_INET6 */ 
	uint16_t sin6_port; /* Transport layer port # */ 
	uint32_t sin6_flowinfo; /* IPv6 flow information */ 
	struct in6_addr sin6_addr; /* IPv6 address */ 
}; 


/* members are in network byte order */
struct sockaddr_in {
  u8_t sin_len;
  u8_t sin_family;
  u16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr {
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};



#define MAX_PORT_NAME 16 
#ifndef IFNAMSIZ
#define IFNAMSIZ  MAX_PORT_NAME
#endif

#define ETH_ALEN        6               /* Octets in one ethernet addr   */
#define ETH_HLEN        14              /* Total octets in header.       */
#define ETH_ZLEN        60              /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500            /* Max. octets in payload        */
#define ETH_FRAME_LEN   1514            /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN     4               /* Octets in the FCS             */

#define ETH_P_802_3     0x0001          /* Dummy type for 802.3 frames  */
#define ETH_P_AX25      0x0002          /* Dummy protocol id for AX.25  */
#define ETH_P_ALL       0x0003          /* Every packet (be careful!!!) */
#define ETH_P_802_2     0x0004          /* 802.2 frames                 */
#define ETH_P_SNAP      0x0005          /* Internal only                */
#define ETH_P_DDCMP     0x0006          /* DEC DDCMP: Internal only     */
#define ETH_P_WAN_PPP   0x0007          /* Dummy type for WAN PPP frames*/
#define ETH_P_PPP_MP    0x0008          /* Dummy type for PPP MP frames */
#define ETH_P_LOCALTALK 0x0009          /* Localtalk pseudo type        */
#define ETH_P_CAN       0x000C          /* Controller Area Network      */
#define ETH_P_PPPTALK   0x0010          /* Dummy type for Atalk over PPP*/
#define ETH_P_TR_802_2  0x0011          /* 802.2 frames                 */
#define ETH_P_MOBITEX   0x0015          /* Mobitex (kaz@cafe.net)       */
#define ETH_P_CONTROL   0x0016          /* Card specific control frames */
#define ETH_P_IRDA      0x0017          /* Linux-IrDA                   */
#define ETH_P_ECONET    0x0018          /* Acorn Econet                 */
#define ETH_P_HDLC      0x0019          /* HDLC frames                  */
#define ETH_P_ARCNET    0x001A          /* 1A for ArcNet :-)            */
#define ETH_P_DSA       0x001B          /* Distributed Switch Arch.     */

/* Ethernet protocol ID's */
#define ETHERTYPE_PUP           0x0200          /* Xerox PUP */
#define ETHERTYPE_SPRITE        0x0500          /* Sprite */
#define ETHERTYPE_IP            0x0800          /* IP */
#define ETHERTYPE_ARP           0x0806          /* Address resolution */
#define ETHERTYPE_REVARP        0x8035          /* Reverse ARP */
#define ETHERTYPE_AT            0x809B          /* AppleTalk protocol */
#define ETHERTYPE_AARP          0x80F3          /* AppleTalk ARP */
#define ETHERTYPE_VLAN          0x8100          /* IEEE 802.1Q VLAN tagging */
#define ETHERTYPE_IPX           0x8137          /* IPX */
#define ETHERTYPE_IPV6          0x86dd          /* IP protocol version 6 */
#define ETHERTYPE_LOOPBACK      0x9000          /* used to test interfaces */


#define ETHER_ADDR_LEN  ETH_ALEN                 /* size of ethernet addr */
#define ETHER_TYPE_LEN  2                        /* bytes in type field */
#define ETHER_CRC_LEN   4                        /* bytes in CRC field */
#define ETHER_HDR_LEN   ETH_HLEN                 /* total octets in header */
#define ETHER_MIN_LEN   (ETH_ZLEN + ETHER_CRC_LEN) /* min packet length */
#define ETHER_MAX_LEN   (ETH_FRAME_LEN + ETHER_CRC_LEN) /* max packet length */


#define MAX_PORTS  12 

#define LLC_PDU_TYPE_I  0       /* first bit */
#define LLC_PDU_TYPE_S  1       /* first two bits */
#define LLC_PDU_TYPE_U  3       /* first two bits */
#define LLC_1_PDU_CMD_UI       0x00     /* Type 1 cmds/rsps */


/* LLC SAP types. */
#define LLC_SAP_NULL    0x00            /* NULL SAP.                    */
#define LLC_SAP_LLC     0x02            /* LLC Sublayer Management.     */
#define LLC_SAP_SNA     0x04            /* SNA Path Control.            */
#define LLC_SAP_PNM     0x0E            /* Proway Network Management.   */
#define LLC_SAP_IP      0x06            /* TCP/IP.                      */
#define LLC_SAP_BSPAN   0x42            /* Bridge Spanning Tree Proto   */
#define LLC_SAP_MMS     0x4E            /* Manufacturing Message Srv.   */
#define LLC_SAP_8208    0x7E            /* ISO 8208                     */
#define LLC_SAP_3COM    0x80            /* 3COM.                        */
#define LLC_SAP_PRO     0x8E            /* Proway Active Station List   */
#define LLC_SAP_SNAP    0xAA            /* SNAP.                        */
#define LLC_SAP_BANYAN  0xBC            /* Banyan.                      */
#define LLC_SAP_IPX     0xE0            /* IPX/SPX.                     */
#define LLC_SAP_NETBEUI 0xF0            /* NetBEUI.                     */
#define LLC_SAP_LANMGR  0xF4            /* LanManager.                  */
#define LLC_SAP_IMPL    0xF8            /* IMPL                         */
#define LLC_SAP_DISC    0xFC            /* Discovery                    */
#define LLC_SAP_OSI     0xFE            /* OSI Network Layers.          */
#define LLC_SAP_LAR     0xDC            /* LAN Address Resolution       */
#define LLC_SAP_RM      0xD4            /* Resource Management          */
#define LLC_SAP_GLOBAL  0xFF            /* Global SAP.                  */

    
/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM   0               /* from KA9Q: NET/ROM pseudo    */
#define ARPHRD_ETHER    1               /* Ethernet 10Mbps              */
#define ARPHRD_EETHER   2               /* Experimental Ethernet        */
#define ARPHRD_AX25     3               /* AX.25 Level 2                */
#define ARPHRD_PRONET   4               /* PROnet token ring            */
#define ARPHRD_CHAOS    5               /* Chaosnet                     */
#define ARPHRD_IEEE802  6               /* IEEE 802.2 Ethernet/TR/TB    */
#define ARPHRD_ARCNET   7               /* ARCnet                       */
#define ARPHRD_APPLETLK 8               /* APPLEtalk                    */
#define ARPHRD_DLCI     15              /* Frame Relay DLCI             */
#define ARPHRD_ATM      19              /* ATM                          */
#define ARPHRD_METRICOM 23              /* Metricom STRIP (new IANA id) */
#define ARPHRD_IEEE1394 24              /* IEEE 1394 IPv4 - RFC 2734    */
#define ARPHRD_EUI64    27              /* EUI-64                       */
#define ARPHRD_INFINIBAND 32            /* InfiniBand                   */

/* ARP protocol opcodes. */
#define ARPOP_REQUEST   1               /* ARP request                  */
#define ARPOP_REPLY     2               /* ARP reply                    */
#define ARPOP_RREQUEST  3               /* RARP request                 */
#define ARPOP_RREPLY    4               /* RARP reply                   */
#define ARPOP_InREQUEST 8               /* InARP request                */
#define ARPOP_InREPLY   9               /* InARP reply                  */
#define ARPOP_NAK       10              /* (ATM)ARP NAK                 */

typedef struct mac_addr
{       
        unsigned char   addr[6];
}MACADDRESS;

        
struct ether_hdr
{
	MACADDRESS  dmac;      /* destination eth addr */
	MACADDRESS  smac;      /* source ether addr    */
	uint16_t type;                 /* packet type ID field */
};
typedef struct {
    /** Six octet holding the MAC address */
    uint8_t octet[6];
} cparser_macaddr_t;

struct ethernet_hdr
{
  u_int8_t  ether_dhost[ETH_ALEN];      /* destination eth addr */
  u_int8_t  ether_shost[ETH_ALEN];      /* source ether addr    */
  u_int16_t ether_type;                 /* packet type ID field */
} __attribute__ ((__packed__));


#if 0
struct ether_addr
{       
  u_int8_t ether_addr_octet[ETH_ALEN];
} __attribute__ ((__packed__));
#endif

typedef struct bridge_id
{       
        uint16_t   prio;
        unsigned char   addr[6];
} __attribute__ ((__packed__))BRIDGEID;

/* Un-numbered PDU format (3 bytes in length) */
struct llc_pdu_un {
        uint8_t dsap;
        uint8_t ssap;
        uint8_t ctrl_1;
} __attribute__ ((__packed__));


typedef uint16_t PORTID;
typedef uint32_t PORT;
typedef uint32_t TIMEOUT;


enum {
	TRUE = 1,
	FALSE = 0
};

enum {
	IF_UP = 1,
	IF_DOWN = 2
};

typedef int TRUTH;
typedef int TIMESTMP;
typedef void * TIMER_ID;

typedef struct mac_hdr {
  MACADDRESS dest;
  MACADDRESS src;
  uint16_t  len8023;
} MACHDR;

typedef struct llc_hdr {
  uint8_t   dsap;
  uint8_t   ssap;
  uint8_t   llc;
}LLCHDR;

typedef enum {
  P2P_FORCE_TRUE,
  P2P_FORCE_FALSE,
  P2P_AUTO,
} ADMIN_P2P_T;

#define S_DISABLED 0
#define S_LISTENING 1
#define S_DISCARDING 1
#define S_LEARNING 2
#define S_FORWARDING 3
#define S_BLOCKING 4

#define STP_ENABLED 1
#define STP_DISABLED 0

#define ADMIN_PORT_PATH_COST_AUTO   0

#define MAX_BITS_PER_BYTE 8
typedef char PORTLIST [MAX_PORTS / MAX_BITS_PER_BYTE];

#define VLAN_INVALID_ID  0X7FFF
#define VLAN_DEFAULT_VLAN_ID 1

#define MAX_VLAN_NAME  8
#define MAX_VLAN_BITS 12
#define MAX_VLAN_IDS  (1 << MAX_VLAN_BITS) 

#define	IN_CLASSA(a)		((((in_addr_t)(a)) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		(0xffffffff & ~IN_CLASSA_NET)
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(a)		((((in_addr_t)(a)) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		(0xffffffff & ~IN_CLASSB_NET)
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(a)		((((in_addr_t)(a)) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		(0xffffffff & ~IN_CLASSC_NET)

#define	IN_CLASSD(a)		((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(a)		IN_CLASSD(a)

#define	IN_EXPERIMENTAL(a)	((((in_addr_t)(a)) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(a)		((((in_addr_t)(a)) & 0xf0000000) == 0xf0000000)

/* Address to accept any incoming messages.  */
#define	INADDR_ANY		((in_addr_t) 0x00000000)
/* Address to send to all hosts.  */
#define	INADDR_BROADCAST	((in_addr_t) 0xffffffff)
/* Address indicating an error return.  */
#define	INADDR_NONE		((in_addr_t) 0xffffffff)

/* Network number for local host loopback.  */
#define	IN_LOOPBACKNET		127
/* Address to loopback in software to local host.  */
#ifndef INADDR_LOOPBACK
# define INADDR_LOOPBACK	((in_addr_t) 0x7f000001) /* Inet 127.0.0.1.  */
#endif

/* Defines for Multicast INADDR.  */
#define INADDR_UNSPEC_GROUP	((in_addr_t) 0xe0000000) /* 224.0.0.0 */
#define INADDR_ALLHOSTS_GROUP	((in_addr_t) 0xe0000001) /* 224.0.0.1 */
#define INADDR_ALLRTRS_GROUP    ((in_addr_t) 0xe0000002) /* 224.0.0.2 */
#define INADDR_MAX_LOCAL_GROUP  ((in_addr_t) 0xe00000ff) /* 224.0.0.255 */


enum ROWSTATUS  {
	ROW_STATE_CREATE_WAIT = 5
};

#define VLAN_IS_NOT_IN_RANGE(vid)  (vid <= 0 || vid > 4096)

#define TAGGED 1
#define UNTAGGED 2
#define FORBIDDEN 3

#define MAXHOSTNAMELEN  64      /* max length of hostname */

#define	ICMP_MINLEN	8				/* abs minimum */

#define ICMP_ECHOREPLY		0	/* Echo Reply			*/
#define ICMP_DEST_UNREACH	3	/* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH	4	/* Source Quench		*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO		8	/* Echo Request			*/
#define ICMP_TIME_EXCEEDED	11	/* Time Exceeded		*/
#define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/
#define ICMP_TIMESTAMP		13	/* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY	14	/* Timestamp Reply		*/
#define ICMP_INFO_REQUEST	15	/* Information Request		*/
#define ICMP_INFO_REPLY		16	/* Information Reply		*/
#define ICMP_ADDRESS		17	/* Address Mask Request		*/
#define ICMP_ADDRESSREPLY	18	/* Address Mask Reply		*/
#define NR_ICMP_TYPES		18


/* Codes for UNREACH. */
#define ICMP_NET_UNREACH	0	/* Network Unreachable		*/
#define ICMP_HOST_UNREACH	1	/* Host Unreachable		*/
#define ICMP_PROT_UNREACH	2	/* Protocol Unreachable		*/
#define ICMP_PORT_UNREACH	3	/* Port Unreachable		*/
#define ICMP_FRAG_NEEDED	4	/* Fragmentation Needed/DF set	*/
#define ICMP_SR_FAILED		5	/* Source Route failed		*/
#define ICMP_NET_UNKNOWN	6
#define ICMP_HOST_UNKNOWN	7
#define ICMP_HOST_ISOLATED	8
#define ICMP_NET_ANO		9
#define ICMP_HOST_ANO		10
#define ICMP_NET_UNR_TOS	11
#define ICMP_HOST_UNR_TOS	12
#define ICMP_PKT_FILTERED	13	/* Packet filtered */
#define ICMP_PREC_VIOLATION	14	/* Precedence violation */
#define ICMP_PREC_CUTOFF	15	/* Precedence cut off */
#define NR_ICMP_UNREACH		15	/* instead of hardcoding immediate value */

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/
#define ICMP_REDIR_NETTOS	2	/* Redirect Net for TOS		*/
#define ICMP_REDIR_HOSTTOS	3	/* Redirect Host for TOS	*/

/* Codes for TIME_EXCEEDED. */
#define ICMP_EXC_TTL		0	/* TTL count exceeded		*/
#define ICMP_EXC_FRAGTIME	1	/* Fragment Reass time exceeded	*/



#define DEFDATALEN      56
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	65468
#define	MAX_DUP_CHK	(8 * 128)
#define MAXWAIT         10
#define PINGINTERVAL    1		/* second */

#define O_QUIET         (1 << 0)

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))





#define MODE_STP 1
#define MODE_RSTP 2

/*FIXME:*/
#define DISABLE_BGP_ANNOUNCE 0



enum STP_DEF_VALUES {
	STP_DEF_PRIORITY = 32768,
	STP_DEF_MAX_AGE = 20,
	STP_DEF_HELLO_TIME = 2,
	STP_DEF_FWD_DELAY = 15,
	STP_DEF_HOLD_COUNT = 6,
	STP_DEF_PORT_PRIO = 128,
	STP_DEF_DESG_COST = 20000,
	STP_DEF_PATH_COST = 20000
};

static const uint8_t br_group_address[ETH_ALEN] = { 0x01, 0x80, 0xc2, 
                                              0x00, 0x00, 0x00 };

int get_port_mac_address (uint32_t port, uint8_t *mac);
int  compare_ether_addr(const uint8_t *addr1, const uint8_t *addr2);
int get_port_oper_state (uint32_t port);
inline int bridge_timer_relation (int fdelay, int max_age, int hello);
int read_port_mac_address (int port, uint8_t *p);
int get_max_ports (void);

#include "libproto.h"
#endif
