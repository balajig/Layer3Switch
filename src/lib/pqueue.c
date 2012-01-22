/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include "common_types.h"

#define MAX_Q_CTRL_BLOCKS    32
#define PKT_RX_ON_SOCK       0x1 

typedef struct _socket_queue_t {
	void            *buf;
	int              len;
	struct  list_head nbuf;
} socket_queue_t;

typedef struct queue_cb 
{
	struct list_head   next;
	struct list_head   queue;
	EVT_T              evt;
	int32_t            sock;
}queue_cb_t;

static int  q_mem_pool_id = -1;

static struct list_head queue_sock_list;


int pqueue_init (void)
{
	INIT_LIST_HEAD (&queue_sock_list);

	q_mem_pool_id = mem_pool_create ("PQUEUE", MAX_Q_CTRL_BLOCKS * sizeof(queue_cb_t), 
                                         MAX_Q_CTRL_BLOCKS, 0);
	if (q_mem_pool_id < 0) {
		return -1;
	}

	return 0;
}


int queue_packet (unsigned long qblk , uint8_t *buf, int len)
{
	queue_cb_t * p = (queue_cb_t *)qcb;
	socket_queue_t  *qbuf = NULL;

	if (!p)
		return -1;

	qbuf = malloc (sizeof(socket_queue_t));

	if (!qbuf)
		return -1;

	INIT_LIST_HEAD (&qbuf->nbuf);

	qbuf->buf = buf;
	qbuf->len = len;

	list_add_tail (&qbuf->nbuf, &p->queue);

	EvtSnd (&p->evt, PKT_RX_ON_SOCK);

	return 0;
}

int dequeue_packet (unsigned long qcb, uint8_t *data, size_t datalen)
{
	queue_cb_t * p = (queue_cb_t *)qcb;

	socket_queue_t  *qbuf = NULL;

	if (!p)
		return -1;

	while (1) {

		int event = 0;
		if (list_empty (&p->queue)) {
			EvtRx (&p->evt, &event, PKT_RX_ON_SOCK);
			if ((event & PKT_RX_ON_SOCK))
				goto packet;

		}
packet:
		if (!list_empty (&p->queue)) {
			qbuf = list_first_entry (&p->queue, socket_queue_t, nbuf);
			if (!qbuf)
				return -1;
			if (datalen > qbuf->len)
				datalen = qbuf->len;
			memcpy (data, qbuf->buf, datalen);
			/*TODO: free buf*/
			list_del (&qbuf->nbuf);
			free (qbuf);
			break;
		}
	}

	return 0;
}
