#include "init.h"

void layer3switch_init (void)
{
        lib_init ();

	mem_init ();

	memp_init ();

	pbuf_init ();

        cli_init ("OpenSwitch");

        port_init ();

        ip_init ();

        bridge_init ();

        vrrp_init ();

        init_task_cpu_usage_moniter_timer ();

	/* Sanity check user-configurable values */
	lwip_sanity_check ();

	/* Modules initialization */
	//stats_init ();
#if !NO_SYS
	sys_init ();
#endif /* !NO_SYS */
	//if_init ();
#if LWIP_SOCKET
	//lwip_socket_init ();
#endif /* LWIP_SOCKET */
	//ip_init ();
#if LWIP_ARP
	etharp_init ();
#endif /* LWIP_ARP */
#if LWIP_RAW
	raw_init ();
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
#if LWIP_AUTOIP
	autoip_init ();
#endif /* LWIP_AUTOIP */
#if LWIP_IGMP
	igmp_init ();
#endif /* LWIP_IGMP */
#if LWIP_DNS
	dns_init ();
#endif /* LWIP_DNS */

#if LWIP_TIMERS
	sys_timeouts_init ();
#endif /* LWIP_TIMERS */

        spawn_pkt_processing_task ();

        start_cli_task ();
}
