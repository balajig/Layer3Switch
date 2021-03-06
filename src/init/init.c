#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/mem.h"
#include "lwip/memp.h"

int lib_init (void);
int cli_init (const char *prmt);
int spawn_pkt_processing_task (void);
int port_init (void);
int bridge_init (void);
int vrrp_init (void);
int dhcp_init (void);
int init_task_cpu_usage_moniter_timer (void);
int start_cli_task (void);
void layer3switch_init (void);
int etharp_init (void);
int rtm_init (void);
int ip_init (void);
void udp_init(void);
void tcp_init(void);
void snmp_init(void);
int dhcpd_init (void);
void igmp_init(void);
void dns_init(void);
void init_sntpd(void);
int telnet_init (void);
void sys_timeouts_init(void);


#ifndef CONFIG_OPENSWITCH_TCP_IP
#define etharp_init()
#define tcp_init()
#define snmp_init()
#define dhcp_init()
#define autoip_init()
#define igmp_init()
#define dns_init()
#endif

void layer3switch_init (void)
{
	mcore_init ();

        lib_init ();

	mem_init ();

	memp_init ();

//	pbuf_init ();

        cli_init ("OpenSwitch");

#ifdef CONFIG_ZEBRA_RTM
	rtm_init ();
#endif  /* RTM_SUPPORT */

        port_init ();

        ip_init ();

        bridge_init ();

        //vrrp_init ();

	/* Modules initialization */
	//stats_init ();
	//if_init ();
#if LWIP_SOCKET
	//lwip_socket_init ();
#endif /* LWIP_SOCKET */
	//ip_init ();
#if LWIP_ARP
	etharp_init ();
#endif /* LWIP_ARP */
#if LWIP_RAW
	//raw_init ();
#endif /* LWIP_RAW */
#if LWIP_UDP
	udp_init ();
#endif /* LWIP_UDP */
#if LWIP_TCP
	tcp_init ();
#endif /* LWIP_TCP */
#if LWIP_SNMP
	snmp_init ();
#endif /* LWIP_SNMP */
	dhcpd_init ();

	dhcp_init ();
#if LWIP_AUTOIP
	//autoip_init ();
#endif /* LWIP_AUTOIP */
#if LWIP_IGMP
	igmp_init ();
#endif /* LWIP_IGMP */
#if LWIP_DNS
	dns_init ();
#endif /* LWIP_DNS */
	init_sntpd ();

#if LWIP_TIMERS
	sys_timeouts_init ();
#endif /* LWIP_TIMERS */
#ifdef CONFIG_TELNET
	telnet_init ();
#endif

        spawn_pkt_processing_task ();

        init_task_cpu_usage_moniter_timer ();

        start_cli_task ();
}
