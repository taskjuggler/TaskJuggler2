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
#include <stdlib.h>

#include "Utility.h"

#include <qdict.h>

static QDict<const char> TZDict;
static bool TZDictReady = FALSE;

static QString UtilityError;

const char*
timezone2tz(const char* tzone)
{
	if (!TZDictReady)
	{
		// Let's start with generic timezones
		TZDict.insert("+1200", "GMT-12:00");
		TZDict.insert("+1100", "GMT-11:00");
		TZDict.insert("+1000", "GMT-10:00");
		TZDict.insert("+0900", "GMT-9:00");
		TZDict.insert("+0800", "GMT-8:00");
		TZDict.insert("+0700", "GMT-7:00");
		TZDict.insert("+0600", "GMT-6:00");
		TZDict.insert("+0500", "GMT-5:00");
		TZDict.insert("+0400", "GMT-4:00");
		TZDict.insert("+0300", "GMT-3:00");
		TZDict.insert("+0200", "GMT-2:00");
		TZDict.insert("+0100", "GMT-1:00");
		TZDict.insert("+0000", "GMT-0:00");
		TZDict.insert("-0100", "GMT+1:00");
		TZDict.insert("-0200", "GMT+2:00");
		TZDict.insert("-0300", "GMT+3:00");
		TZDict.insert("-0400", "GMT+4:00");
		TZDict.insert("-0500", "GMT+5:00");
		TZDict.insert("-0600", "GMT+6:00");
		TZDict.insert("-0700", "GMT+7:00");
		TZDict.insert("-0800", "GMT+8:00");
		TZDict.insert("-0900", "GMT+9:00");
		TZDict.insert("-1000", "GMT+10:00");
		TZDict.insert("-1100", "GMT+11:00");
		TZDict.insert("-1200", "GMT+12:00");
		// Now some conveniance timezones. There will be more in the future.
		TZDict.insert("PST", "GMT8:00");
		TZDict.insert("PDT", "GMT7:00");
		TZDict.insert("MST", "GMT7:00");
		TZDict.insert("MDT", "GMT6:00");
		TZDict.insert("CST", "GMT6:00");
		TZDict.insert("CDT", "GMT5:00");
		TZDict.insert("EST", "GMT5:00");
		TZDict.insert("EDT", "GMT4:00");
		TZDict.insert("GMT", "GMT");
		TZDict.insert("CET", "GMT-1:00");
		TZDict.insert("CEST", "GMT-2:00");

		TZDictReady = TRUE;
	}

	return TZDict[tzone];
}

const QString&
getUtilityError()
{
	return UtilityError;
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
	/* The  ISO  8601:1988 week number of the current year as a decimal
	 * number, range 1 to 53, where  week  1 is  the first week that has at
	 * least 4 days in the current year, and with Monday as the first day
	 * of the week. This is also compliant with DIN 1355. */
	uint week = 0;
	uint weekday1Jan = dayOfWeek(beginOfYear(t), TRUE);
	struct tm* tms = localtime(&t);
	int days = tms->tm_yday;

	if (weekday1Jan > 3)
		days = days - (7 - weekday1Jan);
	else 
		days = days + weekday1Jan;

	if (days < 0)
		if ((weekday1Jan == 4) ||
		   	(dayOfWeek(beginOfYear(beginOfYear(t) - 1), TRUE) == 3))
			week = 53;
		else 
			week = 52;
	else 
		week = days / 7 + 1;

	if ((days > 360) && (week > 52)) 
	{
		if (weekday1Jan == 3)
			week = 53;
		else if (dayOfWeek(sameTimeNextYear(beginOfYear(t)), TRUE) == 4)
			week = 53;
		else
		   	week = 1;
	}

	return week;
}

int 
monthOfYear(time_t t)
{
	struct tm* tms = localtime(&t);
	return tms->tm_mon + 1;
}

int
dayOfWeek(time_t t, bool beginOnMonday)
{
	struct tm* tms = localtime(&t);
	if (beginOnMonday)
		return tms->tm_wday ? tms->tm_wday - 1 : 6;
	else
		return tms->tm_wday;
}

QString
dayOfWeekName(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[64];

	strftime(buf, 63, "%A", tms);
	return buf;
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
beginOfWeek(time_t t, bool beginOnMonday)
{
	struct tm* tms;
	for (tms = localtime(&t) ; tms->tm_wday != (beginOnMonday ? 1 : 0);
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

QString time2rfc(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%Y-%m-%d-%H:%M:%S-%z", tms);
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

QString time2weekday(time_t t)
{
	struct tm* tms = localtime(&t);
	static char buf[128];

	strftime(buf, 127, "%A", tms);
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

time_t
date2time(const QString& date)
{
	int y, m, d, hour, min, sec;
	char tZone[64] = "";
	char* savedTZ = 0;
	bool restoreTZ = FALSE;
	if (sscanf(date, "%d-%d-%d-%d:%d:%d-%s",
			   &y, &m, &d, &hour, &min, &sec, tZone) == 7 ||
		(sec = 0) ||	// set sec to 0
		sscanf(date, "%d-%d-%d-%d:%d-%s",
			   &y, &m, &d, &hour, &min, tZone) == 6)
	{
		const char* tz;
		if ((tz = getenv("TZ")) != 0)
		{
			savedTZ = new char[strlen(tz) + 1];
			strcpy(savedTZ, tz);
		}
		if ((tz = timezone2tz(tZone)) == 0)
			UtilityError.sprintf("Illegal timezone %s", tZone);
		else
		{
			if (setenv("TZ", tz, 1) < 0)
				qFatal("date2time: Ran out of space in environment section.");
			restoreTZ = TRUE;
		}
	}
	else if (sscanf(date, "%d-%d-%d-%d:%d:%d",
				   	&y, &m, &d, &hour, &min, &sec) == 6)
		tZone[0] = '\0';
	else if (sscanf(date, "%d-%d-%d-%d:%d", &y, &m, &d, &hour, &min) == 5)
	{
		sec = 0;
		tZone[0] = '\0';
	}
	else if (sscanf(date, "%d-%d-%d", &y, &m, &d) == 3)
	{
		tZone[0] = '\0';
		hour = min = sec = 0;
	}
	else
	{
		qFatal("Illegal date: %s", date.latin1());
		return 0;
	}

	if (y < 1970)
	{
		UtilityError = "Year must be larger than 1969";
		return 0;
	}
	if (m < 1 || m > 12)
	{
		UtilityError = "Month must be between 1 and 12";
		return 0;
	}
	if (d < 1 || d > 31)
	{
		UtilityError = "Day must be between 1 and 31";
		return 0;
	}

	struct tm t = { sec, min, hour, d, m - 1, y - 1900, 0, 0, -1, 0, 0 };
	time_t localTime = mktime(&t);

	if (restoreTZ)
	{
		if (savedTZ)
		{
			if (setenv("TZ", savedTZ, 1) < 0)
				qFatal("date2time: Ran out of space in environment section.");
			delete [] savedTZ;
		}
		else
			unsetenv("TZ");
	}
	
	return localTime;
}


