/*
 * Utility.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
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

const char* monthAndYear(time_t d);

int daysLeftInMonth(time_t d);

bool isWeekend(time_t d);

int dayOfMonth(time_t d);

int dayOfWeek(time_t d);

time_t midnight(time_t t);

QString time2ISO(time_t t);

#endif

