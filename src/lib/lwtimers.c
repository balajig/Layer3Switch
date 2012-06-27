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
#define    SYS_MAX_TICKS_IN_SEC    100 /*Since tick timer runs for 10ms: 1 sec = 1000ms (10ms * 100) */
#define    TICK_TIMER_GRANULARITY  10  /*10 milli secs*/
#define    SUCCESS                 0
#define    FAILURE                 1

#define MILLISEC_2_NANOSEC(msec)  msec * 1000 * 1000

typedef struct tm_timer
{
	struct  list_head next;
	struct  list_head elist;
	void           *data;
 	void           (*time_out_handler)(void *);
	unsigned int    exp;
	unsigned int    time;
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
static inline void timer_expiry_action (TIMER_T * ptmr);
/****************************************************************/

/************* Private Variable Declaration *********************/
static   int          indx = 0;
static   sync_lock_t  core_timer;
static   LIST_HEAD    (timers_list);
/****************************************************************/

volatile unsigned long ticks = 0;
volatile unsigned long next_expiry = 0;

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

static void debug_timers (void)
{
	TIMER_T *p = NULL;

	printf ("\n\n");
	list_for_each_entry (p, &timers_list, next) {
		printf ("Timer value : %d\n", p->exp);
	}
	printf ("\n\n");
	fflush (stdout);
}

static void timer_add_sort (TIMER_T *new)
{
	TIMER_T *p = NULL;
	int add = 0;

	list_for_each_entry (p, &timers_list, next) {
		if (p->exp > new->exp) {
			add = 1;
			new->next.next = &p->next;
			new->next.prev = p->next.prev;
			p->next.prev->next = &new->next;
			break;
		}
	}
	if (!add)
		list_add_tail (&new->next, &timers_list); 
}

static void timer_add (TIMER_T *p)
{
	if (!next_expiry || next_expiry > p->exp) {
		next_expiry = p->exp;
		list_add (&p->next, &timers_list);
	} else {
	debug_timers ();
		timer_add_sort (p);
	debug_timers ();
	}
}

void * start_timer (unsigned int tick, void *data, void (*handler) (void *), int flags)
{
	TIMER_T  *new = NULL;
	int idx = 0;


	if ( !(idx = alloc_timer_id ())  || 
             !(new = alloc_timer ())) {
		return NULL;
	}

	INIT_LIST_HEAD (&new->next);
	new->idx = idx;
	new->data = data;
	new->time_out_handler = handler;
	new->exp = ticks + tick;
	new->time = tick;

	if (flags)
		new->flags = flags;
	else
		new->flags = TIMER_ONCE;

	timer_lock ();

	timer_add (new);

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
	new->data = data;
	new->time_out_handler = handler;
	new->flags = TIMER_FOREVER;

	*(TIMER_T **)p = new;

	return 0;
}

int mod_timer (void *timer, unsigned int tick)
{
	TIMER_T  *p = (TIMER_T *)timer;

	if (!p)
		return -1;

	p->exp = ticks + tick;
	p->time =  tick;

	timer_lock ();

	timer_add (p);

	timer_unlock ();

	return 0;
}


static int alloc_timer_id (void)
{
	return ++indx;
}

int stop_timer (void *timer)
{
	TIMER_T  *p = (TIMER_T *)timer;

	timer_lock ();

	if (p && p->is_running) {
		p->is_running = 0;
		list_del (&p->next);
	}

	timer_unlock ();
	return 0;
}

int del_timer (void *timer)
{
	TIMER_T  *p = (TIMER_T *)timer;

	timer_lock ();

	if (p->is_running) {
		p->is_running = 0;
		list_del (&p->next);
	}

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
	int diff = next_expiry - ticks;
	if (diff <= 0)  {
		return 1;
	}
        return 0;
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
	return ticks / 100;
}
unsigned int get_ticks (void)
{
	return ticks;
}

unsigned int get_mins (void)
{
	return get_secs() / 60;
}
unsigned int get_hrs (void)
{
	return get_mins () / (24);
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


int init_timer_mgr (void)
{
	tmtaskid_t btmhlftask_id = 0;
	tmtaskid_t task_id = 0;

	if (task_create ("TMRTHF", 99, TSK_SCHED_RR, 32000,
			  tick_clock, NULL, NULL, &task_id) == TSK_FAILURE) {

		return FAILURE;
	}

	timer_lock_create ();

	return SUCCESS;
}


void service_timers (void)
{
     tm_process_tick_and_update_timers ();
}

int timer_restart  (TIMER_T *p)
{
	timer_lock ();

	p->exp = p->time + ticks;
	
	timer_add (p);

	p->is_running = 1;

	timer_unlock ();

	return 0;
}


void handle_expired_timer (TIMER_T *ptmr)
{
	if (ptmr->time_out_handler) {
		ptmr->time_out_handler (ptmr->data);
	}

	if (ptmr->flags & TIMER_ONCE) {
		free_timer (ptmr);
	} 
	else if (ptmr->flags & TIMER_REPEAT) {
		timer_restart (ptmr);
	}
}

void timer_expiry_action (TIMER_T * ptmr)
{
	ptmr->is_running = 0;
	timer_unlock ();
	handle_expired_timer (ptmr);
	timer_lock ();
}


int tm_process_tick_and_update_timers (void)
{
	int i = 0;
	TIMER_T *p, *n;

	list_for_each_entry_safe(p, n, &timers_list, next) {
		int diff = p->exp - ticks;
		if (diff <= 0) {
			list_del (&p->next);
			next_expiry = 0;
			timer_expiry_action (p);
			continue;
		} 
	}

	if (!next_expiry && !list_empty(&timers_list)) {
		p = list_first_entry (&timers_list, TIMER_T , next);
		next_expiry = p->exp;
	}

	return 0;
}
int timer_pending (void *timer)
{
	TIMER_T  *p = (TIMER_T *)timer;

        if (!p)
                return 0;

        return p->is_running;
}

unsigned int timer_get_remaining_time (void *timer)
{
	TIMER_T  *p = (TIMER_T *)timer;
	int t = 0;

        if (!p || !p->is_running) {
                return 0;
        }

        t = p->exp - get_ticks();

        if (t < 0) {
                printf ("\nTIMERS : Oopss negative remainiting time %s\n",__FUNCTION__);
                t = 0;
        }

        return t;
}
