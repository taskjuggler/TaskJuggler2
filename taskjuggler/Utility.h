/*
 * Utility.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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

void initUtility(long dizeSize);

void exitUtility();

void setTimezone(const char* tz);

const QString& getUtilityError();

const char* timezone2tz(const char* tzone);

const char* monthAndYear(time_t d);

int daysLeftInMonth(time_t d);

int weeksLeftInMonth(time_t d);

int monthLeftInYear(time_t d);

int quartersLeftInYear(time_t d);

int daysBetween(time_t t1, time_t t2);

int weeksBetween(time_t t1, time_t t2);

int monthsBetween(time_t t1, time_t t2);

int quartersBetween(time_t t1, time_t t2);

bool isWeekend(time_t d);

time_t midnight(time_t t);

time_t beginOfWeek(time_t t, bool beginOnMonday);

time_t beginOfMonth(time_t t);

time_t beginOfQuarter(time_t t);

time_t beginOfYear(time_t t);

time_t sameTimeNextDay(time_t t);

time_t sameTimeYesterday(time_t t);

time_t sameTimeNextWeek(time_t t);

time_t sameTimeLastWeek(time_t t);

time_t sameTimeNextQuarter(time_t t);

time_t sameTimeNextMonth(time_t t);

time_t sameTimeNextYear(time_t t);

time_t sameTimeLastYear(time_t t);

inline bool isSameDay(time_t d1, time_t d2)
{
	return midnight(d1) == midnight(d2);
}

inline bool isSameWeek(time_t d1, time_t d2, bool beginOnMonday)
{
	return beginOfWeek(d1, beginOnMonday) == beginOfWeek(d2, beginOnMonday);
}

inline bool isSameMonth(time_t d1, time_t d2)
{
	return beginOfMonth(d1) == beginOfMonth(d2);
}

inline bool isSameQuarter(time_t d1, time_t d2)
{
	return beginOfQuarter(d1) == beginOfQuarter(d2);
}

int secondsOfDay(time_t d);

int dayOfMonth(time_t d);

int dayOfYear(time_t d);

int weekOfYear(time_t d, bool beginOnMonday);

int monthOfWeek(time_t d, bool beginOnMonday);

int monthOfYear(time_t d);

int quarterOfYear(time_t d);

int year(time_t d);

int yearOfWeek(time_t d, bool beginOnMonday);

int dayOfWeek(time_t d, bool beginOnMonday);

QString dayOfWeekName(time_t d);

QString time2ISO(time_t t);

QString time2tjp(time_t t);

QString time2rfc(time_t t);

QString time2user(time_t t, const QString& timeFormat);

QString time2time(time_t t);

QString time2date(time_t t);

QString time2weekday(time_t t);

time_t date2time( const QString& );

time_t addTimeToDate(time_t day, time_t t);

#endif

