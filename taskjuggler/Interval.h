/*
 * Interval.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Interval_h_
#define _Interval_h_

#include <time.h>

#include <stdio.h>
#include "Utility.h"

class Interval
{
public:
	Interval(time_t s, time_t e) : start(s), end(e) { }
	Interval(time_t s) : start(s), end(s) { }
	Interval() { start = 0; end = 0; }
	~Interval() { }

	bool isNull() const { return !(start < end); }

	bool contains(time_t date) const
	{
		return (start <= date) && (date <= end);
	}

	bool contains(const Interval& i) const
	{
		return (start <= i.start) && (i.end <= end);
	}
	bool overlap(const Interval& i)
	{
		// Sets the interval to the overlapping interval.
		if (end <= i.start || start >= i.end)
		{
			// Intervals do not overlap.
			end = start - 1;
			return FALSE;
		}
		if (start < i.start)
			start = i.start;
		if (end > i.end)
			end = i.end;
		return TRUE;
	}
	bool overlaps(const Interval& i) const
	{
		return (start <= end && i.start <= i.end &&
				((start <= i.start && i.start <= end) ||
				 (i.start <= start && start <= i.end)));
	}
	bool prepend(const Interval& i)
	{
		if (((i.end + 1) == start) && (i.start < start))
		{
			start = i.start;
			return TRUE;
		}
		return FALSE;
	}
	bool append(const Interval& i)
	{
		if (((end + 1) == i.start) && ((end + 1) <= i.end))
		{
			end = i.end;
			return TRUE;
		}
		return FALSE;
	}
	int compare(const Interval& i) const
	{
		if (end < i.start)
			return -1;	// interval is below i
		else if (i.end < start)
			return 1;	// interval above below i
		else
			return 0;	// interval overlap
	}
	time_t getStart() const { return start; }
	void setStart(time_t s) { start = s; }

	time_t getEnd() const { return end; }
	void setEnd(time_t e) { end = e; }

	time_t getDuration() const { return end >= start ? end - start + 1: 0; }

	const Interval& firstDay()
	{
		start = midnight(start);
		end = sameTimeNextDay(start) - 1;
		return *this;
	}

	const Interval& firstWeek()
	{
		start = beginOfWeek(start);
		end = sameTimeNextWeek(start) - 1;
		return *this;
	}

	const Interval& firstMonth()
	{
		start = beginOfMonth(start);
		end = sameTimeNextMonth(start) - 1;
		return *this;
	}

private:
	/// The start of the time interval.
	time_t start;
	/// The end of the time interval. This value is part of the interval.
	time_t end;
} ;

#endif
