/*
 * VacationList.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "VacationList.h"

int
VacationList::compareItems(QCollection::Item it1, QCollection::Item it2)
{
	Interval* i1 = static_cast<Interval*>(it1);
	Interval* i2 = static_cast<Interval*>(it2);

	if (i1->start == i2->start)
	{
		if (i1->end == i2->end)
			return 0;
		else
			return i1->end - i2->end;
	}
	else
		return i1->start - i2->start;
}

bool
VacationList::isVacationDay(time_t day)
{
	Interval* i;
	for (i = first(); i != 0 && day <= i->getEnd(); i = next())
		if (i->getStart() <= day && day <= i->getEnd())
			return TRUE;

	return FALSE;
}
