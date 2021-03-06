#define MAX_MTU 1500
#define  MAX_MAC_ENTRIES   1024 /*1K Mac-entries*/
#define PKT_PROCESSED 1
#define PKT_DROP 0
#define ETH_ADDR_LEN 6

struct sinfo {
	uint32_t pkt_consumed;
	uint32_t pkt_drop;
	uint32_t cpu_pkt_count;
}gsw_info;

typedef void pkt_t;

struct pkt_q {
	void    *pkt;
	uint32_t len;
	uint32_t port;
};

