/*
 * ShiftSelectionList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ShiftSelectionList.h"
#include "ShiftSelection.h"

int
ShiftSelectionList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	ShiftSelection* s1 = static_cast<ShiftSelection*>(i1);
	ShiftSelection* s2 = static_cast<ShiftSelection*>(i2);

	return s1->period.compare(s2->period);
}

bool
ShiftSelectionList::isOnShift(const Interval& iv) const
{
	/* Check whether any of the defined shift intervals contains the interval
	 * 'iv'. If not return TRUE. If it does, check whether the interval 'iv'
	 * lies within the specified working hours. */
	for (ShiftSelectionListIterator ssli(*this); 
		 *ssli != 0 && iv.getStart() <= (*ssli)->getPeriod().getEnd(); ++ssli)
		if ((*ssli)->getPeriod().contains(iv))
		   return (*ssli)->getShift()->isOnShift(iv);
	return TRUE;
}

bool
ShiftSelectionList::isVacationDay(time_t day) const
{
	for (ShiftSelectionListIterator ssli(*this);
		 *ssli != 0 && day <= (*ssli)->getPeriod().getEnd(); ++ssli)
		if ((*ssli)->isVacationDay(day))
			return TRUE;
	return FALSE;
}

bool
ShiftSelectionList::insert(ShiftSelection* s)
{
	for (ShiftSelection* sl = first(); sl != 0; sl = next())
		if (sl->getPeriod().overlaps(s->getPeriod()))
			return FALSE;
	append(s);
	return TRUE;
}
