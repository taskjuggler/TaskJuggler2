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
weeksLeftInMonth(time_t t)
{
	int left = 0;
	struct tm* tms = localtime(&t);
	for (int m = tms->tm_mon; tms->tm_mon == m;
		 t = sameTimeNextWeek(t), localtime(&t))
	{
		left++;
	}
	return left;
}

int
monthLeftInYear(time_t t)
{
	int left = 0;
	struct tm* tms = localtime(&t);
	for (int m = tms->tm_year; tms->tm_year == m;
		 t = sameTimeNextMonth(t), localtime(&t))
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
weeksBetween(time_t t1, time_t t2)
{
	int days = 0;
	// TODO: Very slow!!!
	for (time_t t = t1; t < t2; t = sameTimeNextWeek(t))
		days++;
	return days;
}

int
monthsBetween(time_t t1, time_t t2)
{
	int months = 0;
	// TODO: Very slow!!!
	for (time_t t = t1; t < t2; t = sameTimeNextMonth(t))
		months++;
	return months;
}

int 
dayOfMonth(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_mday;
}

int
weekOfYear(time_t t)
{
	time_t boy = beginOfYear(t);
	struct tm* tms = localtime(&boy);
	int i;
	for (i = 0; tms->tm_wday != 0; boy = sameTimeNextDay(boy))
		tms = localtime(&boy);
	// If t is in last year's week, we have to do it again for last year.
	if (boy > t)
	{
		boy = beginOfYear(sameTimeLastYear(t));
		tms = localtime(&boy);
		for (i = 0; tms->tm_wday != 0; boy = sameTimeNextDay(boy))
			tms = localtime(&boy);
	}
	tms = localtime(&t);
	return (tms->tm_yday - i) / 7;
}

int 
monthOfYear(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_mon + 1;
}

int
dayOfWeek(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_wday;
}

int
year(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_year + 1900;
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
beginOfWeek(time_t t)
{
	struct tm* tms;
	for (tms = localtime(&t) ; tms->tm_wday != 0;
		 t = sameTimeYesterday(t), tms = localtime(&t))
		;
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
beginOfYear(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_mon = 0;
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
sameTimeYesterday(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_mday--;
	tms->tm_isdst = -1;
	return mktime(tms);
}

time_t
sameTimeNextWeek(time_t t)
{
	struct tm* tms = localtime(&t);
	int weekday = tms->tm_wday;
	do
	{
		t = sameTimeNextDay(t);
		tms = localtime(&t);
	} while (tms->tm_wday != weekday);
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

time_t
sameTimeNextYear(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_year++;
	tms->tm_isdst = -1;
	return mktime(tms);
}

time_t
sameTimeLastYear(time_t t)
{
	struct tm* tms = localtime(&t);
	tms->tm_year--;
	tms->tm_isdst = -1;
	return mktime(tms);
}

QString time2ISO(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%Y-%m-%d %H:%M %Z", tms);
	return buf;
}

QString time2time(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%H:%M %Z", tms);
	return buf;
}

QString time2date(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%Y-%m-%d", tms);
	return buf;
}

time_t
addTimeToDate(time_t day, time_t hour)
{
	day = midnight(day);
	struct tm* tms = localtime(&day);

	tms->tm_hour = hour / (60 * 60);
	tms->tm_min = (hour / 60) % 60;
	tms->tm_sec = hour % 60;

	tms->tm_isdst = -1;
	return mktime(tms);
}
