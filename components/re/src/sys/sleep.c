/**
 * @file sleep.c  System sleep functions
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re_types.h>
#include <re_fmt.h>
#include <re_sys.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
 
#include <sys/select.h>
 

#include  <unistd.h>
/**
 * Blocking sleep for [us] number of microseconds
 *
 * @param us Number of microseconds to sleep
 */
void sys_usleep(unsigned int us)
{
	if (!us)
		return;

 
	 
	do {
		struct timeval tv;

		tv.tv_sec  = us / 1000000;
		tv.tv_usec = us % 1000000;

		(void)select(0, NULL, NULL, NULL, &tv);
	} while (0);
 
}
