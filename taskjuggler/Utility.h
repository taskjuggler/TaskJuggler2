/*
 * Utility.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Utility_h_
#define _Utility_h_

#include <time.h>
#include <qstring.h>

#define MAXTIME 0x7FFFFFFF
#define ONEDAY (60 * 60 * 24)
#define ONEHOUR (60 * 60)

const QString& getUtilityError();

const char* timezone2tz(const char* tzone);

const char* monthAndYear(time_t d);

int daysLeftInMonth(time_t d);

int weeksLeftInMonth(time_t d);

int monthLeftInYear(time_t d);

int daysBetween(time_t t1, time_t t2);

int weeksBetween(time_t t1, time_t t2);

int monthsBetween(time_t t1, time_t t2);

bool isWeekend(time_t d);

time_t midnight(time_t t);

time_t beginOfWeek(time_t t);

time_t beginOfMonth(time_t t);

time_t beginOfYear(time_t t);

time_t sameTimeNextDay(time_t t);

time_t sameTimeYesterday(time_t t);

time_t sameTimeNextWeek(time_t t);

time_t sameTimeNextMonth(time_t t);

time_t sameTimeNextYear(time_t t);

time_t sameTimeLastYear(time_t t);

inline bool isSameDay(time_t d1, time_t d2)
{
	return midnight(d1) == midnight(d2);
}

inline bool isSameWeek(time_t d1, time_t d2)
{
	return beginOfWeek(d1) == beginOfWeek(d2);
}

inline bool isSameMonth(time_t d1, time_t d2)
{
	return beginOfMonth(d1) == beginOfMonth(d2);
}

int secondsOfDay(time_t d);

int dayOfMonth(time_t d);

int weekOfYear(time_t d);

int monthOfYear(time_t d);

int year(time_t d);

int dayOfWeek(time_t d);

QString time2ISO(time_t t);

QString time2tjp(time_t t);

QString time2rfc(time_t t);

QString time2time(time_t t);

QString time2date(time_t t);

QString time2weekday(time_t t);

time_t date2time( const QString& );

time_t addTimeToDate(time_t day, time_t t);

#endif

