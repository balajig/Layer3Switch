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
#include "list.h"

#define    SET_TIMER_MGR_STATE(state)    tmrmgrstate = state

#define    MAX_TIMERS         2000
#define    QUERY_TIMER_EXPIRY  0x1
#define    QUERY_TIMER_INDEX   0x2
#define    TIMER_ONCE  	        0x1
#define    TIMER_REPEAT  	0x2 
#define    TIMER_FOREVER        0x4
#define    TIMER_DELETE         0x8

typedef struct tm_timer
{
	struct  list_head next;
	void           *data;
 	void           (*time_out_handler)(void *);
	unsigned int    rmt;
	unsigned int    exp;
	int 	        idx;
	int	        flags;
	int 		is_running;
}TIMER_T;

/************* Private func Prototype  *********************/
static TIMER_T * alloc_timer    (void);
static  int      alloc_timer_id (void);
void             show_uptime    (void);
void    *        tick_service   (void *unused) ;
void    *        tick_clock     (void *unused);
int              init_timer_mgr (void);
/****************************************************************/

/************* Private Variable Declaration *********************/
static   int          indx = 0;
static   EVT_T        timer_event;
static   sync_lock_t  core_timer;
static   LIST_HEAD    (timers_list);
/****************************************************************/

unsigned long ticks = 0;

static void timer_lock (void)
{
	sync_lock (&core_timer);
}

static void timer_unlock (void)
{
	sync_unlock (&core_timer);
}

static void timer_lock_create (void)
{
	create_sync_lock (&core_timer);
	timer_unlock ();
}

void * start_timer (unsigned int ticks, void *data, void (*handler) (void *), int flags)
{
	TIMER_T  *new = NULL;
	int idx = 0;


	if ( !(idx = alloc_timer_id ())  || 
             !(new = alloc_timer ())) {
		return NULL;
	}

	new->idx = idx;
	new->rmt = ticks;
	new->data = data;
	new->time_out_handler = handler;

	if (flags)
		new->flags = flags;
	else
		new->flags = TIMER_ONCE;


	timer_lock ();

	INC_TIMER_COUNT ();

	timer_unlock ();

	return (void *)new;
}

int setup_timer (void **p, void (*handler) (void *), void *data)
{
	TIMER_T  *new = NULL;
	int idx = 0;
	

	if ( !(idx = alloc_timer_id ())  || 
             !(new = alloc_timer ())) {
		return -1;
	}

	new->idx = idx;
	new->rmt = 0;
	new->data = data;
	new->time_out_handler = handler;

	new->flags = TIMER_FOREVER;

	*(TIMER_T **)p = new;

	timer_lock ();

	timer_unlock ();

	return 0;
}

int mod_timer (void *timer, unsigned int ticks)
{
	TIMER_T  *p = (TIMER_T *)timer;

	if (!p)
		return -1;

	timer_lock ();


	timer_unlock ();


	return 0;
}

int timer_restart  (TIMER_T *p)
{
	timer_lock ();


	timer_unlock ();

	return 0;
}


static int alloc_timer_id (void)
{
	if (TIMER_COUNT() > MAX_TIMERS) {
		return 0;
	}
	return ++indx;
}

int stop_timer (void *timer)
{
	TIMER_T  *p = (TIMER_T *)timer;

	timer_lock ();

	if (p && p->apptimer && p->is_running) {

		p->is_running = 0;
	
		DEC_TIMER_COUNT ();
	}

	timer_unlock ();
	return 0;
}

int del_timer (void *timer)
{
	TIMER_T  *p = (TIMER_T *)timer;

	timer_lock ();

	p->flags |= TIMER_DELETE;

	timer_unlock ();

	return 0;
}

static inline TIMER_T * alloc_timer (void)
{
	return tm_calloc (1, sizeof(TIMER_T));
}

void free_timer (TIMER_T *p) 
{
	tm_free (p, sizeof(*p));
}

static inline int timers_pending_for_service (void)
{
	if (TIMER_COUNT ()) {
	}
	return 0;
service_req:	
        return 1;
}

void update_times ()
{
	timer_lock ();

	ticks++;

	if (timers_pending_for_service ()) {
		service_timers ();
	}

	timer_unlock ();
}

unsigned int get_secs (void)
{
}
unsigned int get_ticks (void)
{
}

unsigned int get_mins (void)
{
}
unsigned int get_hrs (void)
{
}
unsigned int get_timers_count (void)
{
	return TIMER_COUNT();
}

void show_uptime (void)
{
	printf ("Uptime  %d hrs %d mins %d secs %d ticks\n",get_hrs(), 
		 get_mins() % 60, get_secs() % 60, get_ticks() % tm_get_ticks_per_second ());
}
void tm_test_timers_latency (void *unused)
{
	show_uptime ();
	start_timer (1 * tm_get_ticks_per_second (), NULL, tm_test_timers_latency , 0);
}
unsigned int tm_get_ticks_per_second (void) 
{
	return SYS_MAX_TICKS_IN_SEC;
}

unsigned int milli_secs_to_ticks (unsigned int msecs)
{
	return (msecs / TICK_TIMER_GRANULARITY);
}


void * tick_clock (void *unused)
{
	register clock_t    start, end;
	register int        tick = 0;

	for (;;) {

		start = times (NULL);
		tsk_delay (0, MILLISEC_2_NANOSEC (TICK_TIMER_GRANULARITY));
		end = times (NULL);
		
		tick = end - start;

		if (tick <= 0)
			tick = 1;

		while (tick--) {
			update_times (); 
		}
	}
	return NULL;
}

#ifdef TIMER_BTM_HALF
void * tick_service (void *unused) 
{
	int evt = 0;

	while (1) {
		EvtRx (&timer_event, &evt, TMR_SERVE_TIMERS);

		if (evt & TMR_SERVE_TIMERS)
			btm_hlf ();
	}
	return NULL;
}
#endif

static inline void timer_rq_init (void)
{
#ifdef TIMER_BTM_HALF
	INIT_LIST_HEAD (&expd_tmrs);
#endif
	tmrrq.count = 0;
}

int init_timer_mgr (void)
{
	int i = TIMER_WHEEL;
	tmtaskid_t btmhlftask_id = 0;
	tmtaskid_t task_id = 0;

	timer_rq_init ();

	EventInit (&timer_event);

#ifdef TIMER_BTM_HALF
	if (task_create ("TMRBHF", 99, TSK_SCHED_RR, 32000,
	  		  tick_service, NULL, NULL, &btmhlftask_id) == TSK_FAILURE) {
		return FAILURE;
	}
#endif

	if (task_create ("TMRTHF", 99, TSK_SCHED_RR, 32000,
			  tick_clock, NULL, NULL, &task_id) == TSK_FAILURE) {

		return FAILURE;
	}

	timer_lock_create ();

#ifdef TIMER_BTM_HALF
	bh_timer_lock_create ();
#endif

	return SUCCESS;
}


void service_timers (void)
{
        if (tm_process_tick_and_update_timers ()) {
#ifdef TIMER_BTM_HALF
		EvtSnd (&timer_event, TMR_SERVE_TIMERS);
#endif
		return;
	}
}
