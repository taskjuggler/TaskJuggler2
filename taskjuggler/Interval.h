/*
 * ResourceList.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
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
	Interval(time_t s, time_t e) : start(s), end(e) {}
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
			end = start;
			return FALSE;
		}
		if (start < i.start)
			start = i.start;
		if (end > i.end)
			end = i.end;
		return TRUE;
	}
	bool overlaps(const Interval& i)
	{
		return ((start <= i.start && i.start < end) ||
				(i.start <= start && start < i.end));
	}
	bool exclude(const Interval& i)
	{	
		/* Sets the interval to the first non-overlapping segment of
         * the interval. */
		if (end <= i.start || start >= i.end)
		{
			/* Intervals do not overlap, so the result is the current
             * interval. */
			return (start < end);
		}
		if (start < i.start)
			end = i.start;
		else if (end > i.end)
			start = i.end;
		return TRUE;
	}
	bool add(const Interval& i)
	{
		start += i.start;
		end += i.end;
		return (start < end);
	}

	bool append(const Interval& i)
	{
		if (((end + 1) == i.start) && ((end + 1) < i.end))
		{
			end = i.end;
			return TRUE;
		}
		return FALSE;
	}
	int compare(const Interval& i)
	{
		if (end < i.start)
			return -1;	// interval is below i
		else if (i.end < start)
			return 1;	// interval above below i
		else
			return 0;	// interval overlap
	}
	time_t getStart() const { return start; }
	time_t getEnd() const { return end; }
	time_t getDuration() const { return end >= start ? end - start + 1 : 0; }

private:
	time_t start;
	time_t end;
} ;

#endif
