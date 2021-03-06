
#include <stdio.h>
#include <stdint.h>
#include "common_types.h"
#include "list.h"
#include "vlan.h"

#define debug_vlan printf
int stp_create_stp_instance (uint16_t vlan_id, void **p);
int stp_delete_stp_instance (void *p);
int  stp_create_port (void *stp_inst, int port);
int stp_set_state (void *br, int state);
int rstp_create_instance (uint16_t vlan_id, void **p);
int  rstp_create_port (void *bs, int port);
int  stp_delete_port (void *stp_inst, uint32_t port);
int bstp_init (void *bs);
int vlan_spanning_tree_enable_on_vlan (int vlan_id, int mode);

int vlan_spanning_tree_enable_on_vlan (int vlan_id, int mode)
{
        struct vlan_static_entry *p = get_vlan_entry (vlan_id);
	int i = 0;

        if (!p) {
                return -1;
        }
	if (p->stp_enabled && p->stp_mode == mode)
		return 0;

	if (mode == MODE_STP) {
		if (stp_create_stp_instance (vlan_id, &p->stp_instance) < 0) {
			debug_vlan ("Unable to enable STP on vlan %d\n", vlan_id);
			return -1;
		}
		for (i = 1; i <= get_max_ports () ; i++) {
			int res = 0;
			IS_PORT_SET_PORT_LIST (p->egress_ports, i,res)
			if (res) {
				stp_create_port (p->stp_instance, i);	
			}
		}	

		stp_set_state (p->stp_instance, 1);

	} else if (mode == MODE_RSTP) {

		if (rstp_create_instance (vlan_id, &p->rstp_instance) < 0) {
			debug_vlan ("Unable to enable STP on vlan %d\n", vlan_id);
			return -1;
		}


		if (!p->rstp_instance) {
			debug_vlan ("Unable to enable RSTP on vlan %d\n", vlan_id);
			return -1;
		}	

		for (i = 1; i <= get_max_ports () ; i++) {
			int res = 0;
			IS_PORT_SET_PORT_LIST (p->egress_ports, i,res);
			if (res) {
				rstp_create_port (p->rstp_instance, i);
			}
		}	

		bstp_init (p->rstp_instance);

		if (p->stp_instance) {
			stp_set_state (p->stp_instance, 0);
		}
	}

	p->stp_enabled = 1;

	p->stp_mode = mode;

	return 0;
}

int vlan_spanning_tree_disable_on_vlan (int vlan_id, int mode)
{
        struct vlan_static_entry *p = get_vlan_entry (vlan_id);

        if (!p) {
                return -1;
        }
	if (!p->stp_enabled)
		return 0;

	p->stp_enabled = 0;

	if (mode == MODE_STP) {
		if (stp_delete_stp_instance (p->stp_instance) < 0) {
			debug_vlan ("Unable to disable STP on vlan %d\n", vlan_id);
			return -1;
		}	
		p->stp_instance = NULL;
	} else if (mode == MODE_RSTP) {

		p->rstp_instance = NULL;

		if (p->stp_instance) {
			int i = 0;
			p->stp_enabled = 1;
			p->stp_mode = MODE_STP;
			for (i = 1; i <= get_max_ports () ; i++) {
				int res = 0;
				IS_PORT_SET_PORT_LIST (p->egress_ports, i,res);
				if (res) {
					stp_create_port (p->stp_instance, i);	
				}
			}	
			stp_set_state (p->stp_instance, 1);
		}
	}
	return 0;
}

int vlan_create_stp_port (struct vlan_static_entry *p, int port_no)
{
	if (p->stp_mode == MODE_STP) {
		if (stp_create_port (p->stp_instance, port_no) < 0) {
			return -1;
		}
	} else if (p->stp_mode == MODE_RSTP) {
#if 0
		rstp8021w_port_create (p->rstp_instance, port_no);
#endif
	}
	return 0;
}

int vlan_delete_stp_port (void *stp_inst, int port_no)
{
	if (stp_delete_port (stp_inst, port_no) < 0) {
		return -1;
	}
	return 0;
}

int vlan_get_this_bridge_stp_mode  (int vlanid)
{
	struct vlan_static_entry *p = get_vlan_entry (vlanid);
	
	if (p) {
		return p->stp_mode; 
	}
	return -1;
}

void * vlan_get_spanning_tree_instance (int vlan_id, int mode)
{
        struct vlan_static_entry *p = get_vlan_entry (vlan_id);

        if (!p) {
                return NULL;
        }
	if (!p->stp_enabled)
		return NULL;

	if (mode == MODE_STP) {
		return p->stp_instance;
	} else if (mode == MODE_RSTP) {
		return p->rstp_instance;
	}
	return NULL;
}

