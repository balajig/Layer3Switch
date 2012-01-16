/* 
 *  Description: Network Trouble shooter 
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>      
#include <sys/stat.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <getopt.h>

#include "common_types.h"
#include "ifmgmt.h"
#include "nt.h"
#include <sys/ioctl.h>

#define _PATH_PROCNET_DEV "/proc/net/dev"


extern char switch_mac[];
int set_ip_address (uint32_t ifindex, uint32_t ipaddress, uint32_t ipmask);
int fetch_and_update_if_info (if_t *ife);
int route_add_if (unsigned char *ipaddr, unsigned char masklen, if_t *netif);
int read_interfaces (void);

static int idx = 0;

int get_max_ports (void)
{
	return idx;
}

static int create_raw_sock (char *name)
{
	int sd = -1;
        struct ifreq ifr;
	struct sockaddr_ll addr;
	int  fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0)
		return (-1);

	memset (&addr, 0, sizeof(addr));

	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	ioctl(fd, SIOCGIFINDEX, (char *)&ifr);

	close (fd);

	if ((sd =socket (AF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror ("SOCKET");
		port_cdb[idx].platform = (void *)-1;
		return -1;
	}

	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	addr.sll_protocol = htons(ETH_P_ALL);	

	if (bind (sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_ll)) < 0)
		perror ("bind");

	port_cdb[idx].platform = (void *)sd;

	return 0;
}

static if_t *add_if_info(char *name)
{
    strncpy(IF_DESCR(idx + 1), name, IFNAMSIZ);

    create_raw_sock (name);

     fetch_and_update_if_info (IF_INFO(idx + 1));

    return IF_INFO(idx + 1);
}
static char *get_name(char *name, char *p)
{
    while (isspace(*p))
        p++;
    while (*p) {
        if (isspace(*p))
            break;
        if (*p == ':') {        /* could be an alias */
            char *dot = p, *dotname = name;
            *name++ = *p++;
            while (isdigit(*p))
                *name++ = *p++;
            if (*p != ':') {    /* it wasn't, backup */
                p = dot;
                name = dotname;
            }
            if (*p == '\0')
                return NULL;
            p++;
            break;
        }
        *name++ = *p++;
    }
    *name++ = '\0';
    return p;
}


static int if_readconf(void)
{
	int numreqs = 30;
	struct ifconf ifc;
	struct ifreq *ifr;
	int n, err = -1;
	int   skfd      = socket(AF_INET, SOCK_DGRAM, 0);

	if (skfd < 0)
		return (-1);

	ifc.ifc_buf = NULL;
	for (;;) {
		ifc.ifc_len = sizeof(struct ifreq) * numreqs;
		ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);

		if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
			perror("SIOCGIFCONF");
			goto out;
		}
		if (ifc.ifc_len == (int)(sizeof(struct ifreq) * numreqs)) {
			/* assume it overflowed and try again */
			numreqs += 10;
			continue;
		}
		break;
	}

	ifr = ifc.ifc_req;
	for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq), ifr++) {
		add_if_info(ifr->ifr_name);
	}
	err = 0;

out:
	free(ifc.ifc_buf);
	return err;
}

static int if_readlist_proc(char *target)
{
	static int proc_read;
	FILE *fh;
	char buf[512];
	if_t *ife;
	int err;

	if (proc_read)
		return 0;
	if (!target)
		proc_read = 1;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		fprintf(stderr, ("Warning: cannot open %s (%s). Limited output.\n"),
				_PATH_PROCNET_DEV, strerror(errno));
		return if_readconf();
	}
	fgets(buf, sizeof buf, fh); /* eat line */
	fgets(buf, sizeof buf, fh);

	err = 0;
	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[IFNAMSIZ];
		s = get_name(name, buf);
		if (!strncmp (name, "lo", strlen ("lo")))
			continue;
		ife = add_if_info(name);
		idx++;
		if (target && !strcmp(target,name))
			break;
	}
	if (ferror(fh)) {
		perror(_PATH_PROCNET_DEV);
		err = -1;
		proc_read = 0;
	}

	fclose(fh);
	return err;
}

static int if_readlist(void)
{
	int err = if_readlist_proc (NULL);
	if (err < 0)
		err = if_readconf();
	return err;
}

int fetch_and_update_if_info (if_t *ife)
{
	struct ifreq ifr;
	char *ifname = ife->ifDescr; 
	int  fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0)
		return (-1);

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFINDEX, (char *)&ifr) == 0) {
		update_linux_if_map (idx, ifr.ifr_ifindex);
		ife->ifIndex = idx;
	}

	if (ioctl(fd, SIOCGIFMTU, (char *)&ifr) == 0)
		ife->ifMtu = ifr.ifr_mtu;


	if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) == 0) {
		struct ifreq ifr_tmp;
		struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
		struct sockaddr_in *mask = (struct sockaddr_in *)&ifr_tmp.ifr_addr;

		strncpy(ifr_tmp.ifr_name, ifname, sizeof(ifr.ifr_name));

		if (ioctl(fd, SIOCGIFNETMASK, &ifr_tmp) == 0) {
			//ife->ipv4_netmask.s_addr = mask->sin_addr.s_addr;
			;
		} 
		if (!set_ip_address (idx + 1, sin->sin_addr.s_addr,  mask->sin_addr.s_addr))
		{
			if(strncmp (ifname, "lo", strlen ("lo"))) {
				uint8_t addr[4];
				uint32_2_ipstring (sin->sin_addr.s_addr, addr);
				route_add_if (addr, u32ip_masklen (mask->sin_addr.s_addr),IF_INFO(idx + 1));
			}
		}

	}  	

	if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)  {
		ife->ifAdminStatus = (ifr.ifr_flags & IFF_UP)? IF_UP: IF_DOWN;
		ife->ifOperStatus  = (ifr.ifr_flags & IFF_RUNNING)?IF_UP:IF_DOWN;
	} 

	if (ioctl (fd, SIOCGIFHWADDR, &ifr) == 0) {
		unsigned char  *p = (unsigned char *)ifr.ifr_hwaddr.sa_data;
		int i = 0;
		while (i < 6) {
			ife->ifPhysAddress[i] = switch_mac[i];
			i++;
		}
	}

	close(fd);

	return 0;
}

int read_interfaces (void)
{
	if (if_readlist () < 0)
		return -1;
	return 0;
}

int make_if_up (if_t *p)
{
	struct ifreq ifr;
	int  fd = -1;

	/*MUST be ROOT to make If UP*/
	if (!getuid ()) {
		return -1;
	}
	
	fd =  socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return (-1);

	memset (&ifr, 0, sizeof(ifr));

	strncpy(ifr.ifr_name, p->ifDescr, sizeof(ifr.ifr_name));

	ifr.ifr_flags |= IFF_UP;
	ifr.ifr_flags |= IFF_RUNNING;

	/*make the interface UP and Running*/
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
		return -1;

	/*Read and update the interface states*/
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)  {
		p->ifAdminStatus = (ifr.ifr_flags & IFF_UP)? IF_UP: IF_DOWN;
		p->ifOperStatus  = (ifr.ifr_flags & IFF_RUNNING)?IF_UP:IF_DOWN;
	}

	return 0;
}
