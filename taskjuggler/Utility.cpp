/*
 * Utility.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>
#include "Utility.h"

#include <qdict.h>

static QDict<const char> TZDict;
static bool TZDictReady = FALSE;

const char*
timezone2tz(const char* tzone)
{
	if (!TZDictReady)
	{
		TZDict.insert("PST", "GMP8:00");
		TZDict.insert("PDT", "GMT7:00");
		TZDict.insert("MST", "GMT7:00");
		TZDict.insert("MDT", "GMT6:00");
		TZDict.insert("CST", "GMT6:00");
		TZDict.insert("CDT", "GMT5:00");
		TZDict.insert("EST", "GMT5:00");
		TZDict.insert("EDT", "GMT4:00");
		TZDict.insert("CET", "GMT-1:00");
		TZDict.insert("CEST", "GMT-2:00");

		TZDictReady = TRUE;
	}

	return TZDict[tzone];
}

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
secondsOfDay(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_sec + tms->tm_min * 60 + tms->tm_hour * 3600;
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
	return (tms->tm_yday - i) / 7 + 1;
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

QString time2tjp(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%Y-%m-%d-%H:%M:%S-%Z", tms);
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


time_t dateToTime( const QString& date)
{
        int y, m, d, hour, min;
        if (date.find(':') == -1)
        {
                sscanf(date, "%d-%d-%d", &y, &m, &d);
                hour = min = 0;
        }
        else
                sscanf(date, "%d-%d-%d-%d:%d", &y, &m, &d, &hour, &min);

        if (y < 1970)
        {
                y = 1970;
        }
        if (m < 1 || m > 12)
        {
                m = 1;
        }
        if (d < 1 || d > 31)
        {
                d = 1;
        }

        struct tm t = { 0, min, hour, d, m - 1, y - 1900, 0, 0, -1, 0, 0 };
        time_t localTime = mktime(&t);

        return localTime;
}

