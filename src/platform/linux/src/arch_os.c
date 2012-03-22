/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include "common_types.h"
#include "err.h"

tmtaskid_t tsk_selfid ();
tmtask_t           * get_tsk_info_frm_id (tmtaskid_t tskid);
void * tsk_wrap (void *ptskarg);

int create_sync_lock (sync_lock_t *slock)
{
	if (!slock)
		return -1;
	if (sem_init(slock, 0, 0) < 0) {
		perror ("SEM_INIT: ");
		return -1;
	}
	return 0;
}

int destroy_sync_lock (sync_lock_t *slock)
{
	if (!slock)
		return -1;
	if (sem_destroy(slock) < 0) {
		perror ("SEM_DESTROY: ");
		return -1;
	}
	return 0;
}


int sync_lock (sync_lock_t *slock)
{
	while (sem_wait (slock) < 0)  {
		/*signal interrupts*/
		if (errno == EINTR) {
			continue;
		}
	}

	return 0;
}

int sync_lock_timed_wait (sync_lock_t *slock, int secs, int nanosecs)
{
	struct timespec abs_timeout;

	abs_timeout.tv_sec = secs;
	abs_timeout.tv_nsec = nanosecs;

	while (sem_timedwait (slock, &abs_timeout) < 0)  {
		/*signal interrupts*/
		if (errno == EINTR) {
			continue;
		}
		if (errno == ETIMEDOUT) {
			return -1;
		}
	}

	return 0;
}

int sync_unlock (sync_lock_t *slock)
{
	if (sem_post (slock) < 0) {
		perror ("SEM_POST: ");
		return -1;
	}
	return 0;
}


retval_t deinit_tsk_attr (tmtask_t * ptskinfo)
{
    pthread_attr_init (&ptskinfo->tsk_attr);

    return TSK_SUCCESS;
}

retval_t
deinit_tsk_mtx_and_cond (tmtask_t * ptskinfo)
{
    pthread_cond_destroy (&ptskinfo->tsk_cnd);

    pthread_mutex_destroy (&ptskinfo->tsk_mtx);

    pthread_mutex_destroy (&ptskinfo->evt_mtx);

    return TSK_SUCCESS;
}

retval_t
start_task (tmtask_t * ptskinfo, tmtaskid_t * ptskid)
{
    if (pthread_create (ptskid, &ptskinfo->tsk_attr, ptskinfo->start_routine,
                        (void *) ptskinfo->tskarg))
    {
        return TSK_FAILURE;
    }
    return TSK_SUCCESS;
}

tmtaskid_t tsk_selfid ()
{
    return pthread_self ();
}

void
tsk_cancel (tmtaskid_t task_id)
{
    pthread_cancel (task_id);
}

retval_t
init_tsk_mtx_and_cond (tmtask_t * ptskinfo)
{
    pthread_cond_init (&ptskinfo->tsk_cnd, NULL);

    pthread_mutex_init (&ptskinfo->tsk_mtx, NULL);

    pthread_mutex_init (&ptskinfo->evt_mtx, NULL);

    return TSK_SUCCESS;

}

retval_t
init_tsk_attr (tmtask_t * ptskinfo)
{
    struct sched_param  param;

    pthread_attr_init (&ptskinfo->tsk_attr);

    pthread_attr_setstacksize (&ptskinfo->tsk_attr, ptskinfo->stksze);

    pthread_attr_setschedpolicy (&ptskinfo->tsk_attr, ptskinfo->schedalgo);

    param.sched_priority = ptskinfo->prio;

    pthread_attr_setschedparam (&ptskinfo->tsk_attr, &param);

    switch (getuid ())
    {
        case 0:                /* For Root User */
            pthread_attr_setinheritsched (&ptskinfo->tsk_attr,
                                          PTHREAD_EXPLICIT_SCHED);
            break;
        default:                /*For Other Users */
            pthread_attr_setinheritsched (&ptskinfo->tsk_attr,
                                          PTHREAD_INHERIT_SCHED);
            break;
    }
    return TSK_SUCCESS;
}

void tsk_delay (int secs, int nsecs)
{
    struct timespec     delay_tmr;

    delay_tmr.tv_sec = secs;

    delay_tmr.tv_nsec = nsecs;

    nanosleep (&delay_tmr, NULL);
}

void tsk_sleep (int secs)
{
    sleep (secs);
}

void tsk_mdelay (int msecs)
{
    usleep (msecs * 1000);
}

pid_t get_tsk_pid ()
{
    return syscall (SYS_gettid);
}

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
int EvtLock (EVT_T *evt)
{
	if (!evt)
		return -1;

	return pthread_mutex_lock (&evt->evt_mtx);
}

int EvtUnLock (EVT_T *evt)
{
	if (!evt)
		return -1;

	return pthread_mutex_unlock (&evt->evt_mtx);
}

int EvtWaitOn (EVT_T *evt)
{
	if (!evt)
		return -1;
	return pthread_cond_wait (&evt->evt_cnd, &evt->evt_mtx);
}

int EvtWaitOnTimed (EVT_T *evt, int secs, int nsecs)
{
	struct timespec ts;

	if (!evt)
		return -1;

	clock_gettime(CLOCK_REALTIME, &ts);

	ts.tv_sec  += secs;
	ts.tv_nsec += nsecs;

	return pthread_cond_timedwait (&evt->evt_cnd, &evt->evt_mtx, &ts);
}


void EvtSignal (EVT_T *evt)
{
	if (!evt)
		return;
	pthread_cond_signal (&evt->evt_cnd);
	pthread_mutex_unlock (&evt->evt_mtx);
}
