/*
 * Utility.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>
#include "Utility.h"

const char*
monthAndYear(time_t t)
{
	struct tm* tms = localtime(&t);
	static char s[32];
	strftime(s, sizeof(s), "%b %Y", tms);
	return s;
}

bool
isWeekend(time_t t)
{
	struct tm* tms = localtime(&t);
	return (tms->tm_wday < 1 || tms->tm_wday > 5);
}

int
daysLeftInMonth(time_t t)
{
	int left = 0;
	struct tm* tms = localtime(&t);
	for (int m = tms->tm_mon; tms->tm_mon == m;
		 t = sameTimeNextDay(t), localtime(&t))
	{
		left++;
	}
	return left;
}

int
daysBetween(time_t t1, time_t t2)
{
	int days = 0;
	// TODO: Very slow!!!
	for (time_t t = t1; t < t2; t = sameTimeNextDay(t))
		days++;
	return days;
}

int 
dayOfMonth(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_mday;
}

int
dayOfWeek(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_isdst = -1;
	return tms->tm_wday;
}

time_t
midnight(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_sec = tms->tm_min = tms->tm_hour = 0;
	tms->tm_isdst = -1;
	return mktime(tms);
}

time_t
beginOfMonth(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_mday = 1;
	tms->tm_sec = tms->tm_min = tms->tm_hour = 0;
	tms->tm_isdst = -1;
	return mktime(tms);
}

time_t
sameTimeNextDay(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_mday++;
	tms->tm_isdst = -1;
	return mktime(tms);
}

time_t
sameTimeNextMonth(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_mon++;
	tms->tm_isdst = -1;
	return mktime(tms);
}

const char* time2ISO(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%Y-%m-%d %H:%M %Z", tms);
	return buf;
}
