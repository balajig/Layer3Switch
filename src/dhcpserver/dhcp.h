/* A domain search list element. */
struct domain_search_list {
	struct domain_search_list *next;
	char *domain;
	TIME rcdate;
};

/* Option tag structures are used to build chains of option tags, for
   when we're sure we're not going to have enough of them to justify
   maintaining an array. */

struct option_tag {
	struct option_tag *next;
	u_int8_t data [1];
};
/* A dhcp packet and the pointers to its option values. */
struct packet {
	struct dhcp_packet *raw;
	int refcnt;
	unsigned packet_length;
	int packet_type;

	unsigned char dhcpv6_msg_type;		/* DHCPv6 message type */

	/* DHCPv6 transaction ID */
	unsigned char dhcpv6_transaction_id[3];

	/* DHCPv6 relay information */
	unsigned char dhcpv6_hop_count;
	struct in6_addr dhcpv6_link_address;
	struct in6_addr dhcpv6_peer_address;

	/* DHCPv6 packet containing this one, or NULL if none */
	struct packet *dhcpv6_container_packet;

	int options_valid;
	int client_port;
	struct iaddr client_addr;
	struct interface_info *interface;	/* Interface on which packet
						   was received. */
	struct hardware *haddr;		/* Physical link address
					   of local sender (maybe gateway). */

	/* Information for relay agent options (see
	   draft-ietf-dhc-agent-options-xx.txt). */
	u_int8_t *circuit_id;		/* Circuit ID of client connection. */
	int circuit_id_len;
	u_int8_t *remote_id;		/* Remote ID of client. */
	int remote_id_len;

	int got_requested_address;	/* True if client sent the
					   dhcp-requested-address option. */

	struct shared_network *shared_network;
	struct option_state *options;

#if !defined (PACKET_MAX_CLASSES)
# define PACKET_MAX_CLASSES 5
#endif
	int class_count;
	struct class *classes [PACKET_MAX_CLASSES];

	int known;
	int authenticated;

	/* If we stash agent options onto the packet option state, to pretend
	 * options we got in a previous exchange were still there, we need
	 * to signal this in a reliable way.
	 */
	isc_boolean_t agent_options_stashed;

	/*
	 * ISC_TRUE if packet received unicast (as opposed to multicast).
	 * Only used in DHCPv6.
	 */
	isc_boolean_t unicast;
};

/* A dhcp lease declaration structure. */
struct lease {
	OMAPI_OBJECT_PREAMBLE;
	struct lease *next;
	struct lease *n_uid, *n_hw;

	struct iaddr ip_addr;
	TIME starts, ends, sort_time;
	char *client_hostname;
	struct binding_scope *scope;
	struct host_decl *host;
	struct subnet *subnet;
	struct pool *pool;
	struct class *billing_class;
	struct option_chain_head *agent_options;

	struct executable_statement *on_expiry;
	struct executable_statement *on_commit;
	struct executable_statement *on_release;

	unsigned char *uid;
	unsigned short uid_len;
	unsigned short uid_max;
	unsigned char uid_buf [7];
	struct hardware hardware_addr;

	u_int8_t flags;
#       define STATIC_LEASE		1
#	define BOOTP_LEASE		2
#	define RESERVED_LEASE		4
#	define MS_NULL_TERMINATION	8
#	define ON_UPDATE_QUEUE		16
#	define ON_ACK_QUEUE		32
#	define ON_QUEUE			(ON_UPDATE_QUEUE | ON_ACK_QUEUE)
#	define UNICAST_BROADCAST_HACK	64
#	define ON_DEFERRED_QUEUE	128

/* Persistent flags are to be preserved on a given lease structure. */
#       define PERSISTENT_FLAGS		(ON_ACK_QUEUE | ON_UPDATE_QUEUE)
/* Ephemeral flags are to be preserved on a given lease (copied etc). */
#	define EPHEMERAL_FLAGS		(MS_NULL_TERMINATION | \
					 UNICAST_BROADCAST_HACK | \
					 RESERVED_LEASE | \
					 BOOTP_LEASE)

	/*
	 * The lease's binding state is its current state.  The next binding
	 * state is the next state this lease will move into by expiration,
	 * or timers in general.  The desired binding state is used on lease
	 * updates; the caller is attempting to move the lease to the desired
	 * binding state (and this may either succeed or fail, so the binding
	 * state must be preserved).
	 *
	 * The 'rewind' binding state is used in failover processing.  It
	 * is used for an optimization when out of communications; it allows
	 * the server to "rewind" a lease to the previous state acknowledged
	 * by the peer, and progress forward from that point.
	 */
	binding_state_t binding_state;
	binding_state_t next_binding_state;
	binding_state_t desired_binding_state;
	binding_state_t rewind_binding_state;

	struct lease_state *state;

	/*
	 * 'tsfp' is more of an 'effective' tsfp.  It may be calculated from
	 * stos+mclt for example if it's an expired lease and the server is
	 * in partner-down state.  'atsfp' is zeroed whenever a lease is
	 * updated - and only set when the peer acknowledges it.  This
	 * ensures every state change is transmitted.
	 */
	TIME tstp;	/* Time sent to partner. */
	TIME tsfp;	/* Time sent from partner. */
	TIME atsfp;	/* Actual time sent from partner. */
	TIME cltt;	/* Client last transaction time. */
	u_int32_t last_xid; /* XID we sent in this lease's BNDUPD */
	struct lease *next_pending;

	/*
	 * A pointer to the state of the ddns update for this lease.
	 * It should be set while the update is in progress and cleared
	 * when the update finishes.  It can be used to cancel the
	 * update if we want to do a different update.
	 */
	struct dhcp_ddns_cb *ddns_cb;
};

struct lease_state {
	struct lease_state *next;

	struct interface_info *ip;

	struct packet *packet;	/* The incoming packet. */

	TIME offered_expiry;

	struct option_state *options;
	struct data_string parameter_request_list;
	int max_message_size;
	unsigned char expiry[4], renewal[4], rebind[4];
	struct data_string filename, server_name;
	int got_requested_address;
	int got_server_identifier;
	struct shared_network *shared_network;	/* Shared network of interface
						   on which request arrived. */

	u_int32_t xid;
	u_int16_t secs;
	u_int16_t bootp_flags;
	struct in_addr ciaddr;
	struct in_addr siaddr;
	struct in_addr giaddr;
	u_int8_t hops;
	u_int8_t offer;
	struct iaddr from;
};




struct pool {
	struct pool *next;
	struct group *group;
	struct shared_network *shared_network;
	struct permit *permit_list;
	struct permit *prohibit_list;
	struct lease *active;
	struct lease *expired;
	struct lease *free;
	struct lease *backup;
	struct lease *abandoned;
	struct lease *reserved;
	TIME next_event_time;
	int lease_count;
	int free_leases;
	int backup_leases;
	int index;
	TIME valid_from;        /* deny pool use before this date */
	TIME valid_until;       /* deny pool use after this date */
};

struct shared_network {
	struct shared_network *next;
	char *name;
#define SHARED_IMPLICIT	  1 /* This network was synthesized. */
	int flags;
	struct subnet *subnets;
	struct interface_info *interface;
	struct pool *pools;
	struct ipv6_pool **ipv6_pools;		/* NULL-terminated array */
	int last_ipv6_pool;			/* offset of last IPv6 pool
						   used to issue a lease */
	struct group *group;
#if defined (FAILOVER_PROTOCOL)
	dhcp_failover_state_t *failover_peer;
#endif
};

struct subnet {
	struct subnet *next_subnet;
	struct subnet *next_sibling;
	struct shared_network *shared_network;
	struct interface_info *interface;
	struct iaddr interface_address;
	struct iaddr net;
	struct iaddr netmask;
	int prefix_len;			/* XXX: currently for IPv6 only */
	struct group *group;
};
