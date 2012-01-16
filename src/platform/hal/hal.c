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
#include "ifmgmt.h"

int hal_interface_down(struct interface *fp);
int hal_interface_up(struct interface *fp);
extern int make_if_down (struct interface *fp);
extern int make_if_up (struct interface *fp);

int hal_interface_down (struct interface *fp)
{
	if (make_if_down (fp) < 0) {
		printf ("Unable to make interface down\n");
		return -1;
	}
	return 0;
}

int hal_interface_up (struct interface *fp)
{
	if (make_if_up (fp) < 0) {
		printf ("Unable to make interface UP\n");
		return -1;
	}
	return 0;
}
