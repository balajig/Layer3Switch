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
#include "err.h"

int EventInit (EVT_T *p)
{
	if (!p)
		return -1;

	pthread_cond_init (&p->evt_cnd, NULL);
	pthread_mutex_init (&p->evt_mtx, NULL);
	return 0;
}

int EventDeInit (EVT_T *p)
{
	if (!p)
		return -1;

	pthread_cond_destroy (&p->evt_cnd);
	pthread_mutex_destroy (&p->evt_mtx);
	return 0;
}


int EvtRx (EVT_T *evt, int *pevent, int event)
{
	pthread_mutex_lock (&evt->evt_mtx);

	while (1)
	{
		if (evt->event & event)
		{
			*pevent = evt->event;
			evt->event &= 0;
			pthread_mutex_unlock (&evt->evt_mtx);
			return 0;
		}
		pthread_cond_wait (&evt->evt_cnd, &evt->evt_mtx);
	}

	return -1;
}

int EvtRx_timed_wait (EVT_T *evt, int *pevent, int event, int secs, int nsecs)
{
	struct timespec ts;
	int    err = 0;

	pthread_mutex_lock (&evt->evt_mtx);

	clock_gettime(CLOCK_REALTIME, &ts);

	ts.tv_sec  += secs;
	ts.tv_nsec += nsecs;

	while (1)
	{
		if (evt->event & event)
		{
			*pevent = evt->event;
			evt->event &= 0;
			pthread_mutex_unlock (&evt->evt_mtx);
			return 0;
		}
		err =  pthread_cond_timedwait (&evt->evt_cnd, &evt->evt_mtx, &ts);
		if (err == ETIMEDOUT) {
			pthread_mutex_unlock (&evt->evt_mtx);
			return ERR_TIMEOUT;
		}
	}

	return -1;
}


void EvtSnd (EVT_T *evt, int event)
{

	pthread_mutex_lock (&evt->evt_mtx);

	evt->event |= event;

	pthread_cond_signal (&evt->evt_cnd);
	pthread_mutex_unlock (&evt->evt_mtx);

	return;
}
