/*
 * ShiftList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ShiftList.h"

int
ShiftSelectionList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	ShiftSelection* r1 = static_cast<ShiftSelection*>(i1);
	ShiftSelection* r2 = static_cast<ShiftSelection*>(i2);
	return r1->period.compare(r2->period);
}

Shift*
ShiftList::getShift(const QString& id)
{
	for (Shift* r = first(); r != 0; r = next())
		if (r->getId() == id)
			return r;

	return 0;
}

Shift::Shift(Project* prj, const QString& i, const QString& n, Shift* p) :
	CoreAttributes(prj, i, n, p)
{
	if (p)
	{
		p->sub.append(this);

		// Inherit start values from parent resource.
		for (int i = 0; i < 7; i++)
		{
			workingHours[i] = new QPtrList<Interval>();
			workingHours[i]->setAutoDelete(TRUE);
			for (Interval* iv = p->workingHours[i]->first(); iv != 0;
				 iv = p->workingHours[i]->next())
				workingHours[i]->append(new Interval(*iv));
		}
	}
	else
	{
		// Sunday
		workingHours[0] = new QPtrList<Interval>();

		// Monday
		workingHours[1] = new QPtrList<Interval>();
		workingHours[1]->setAutoDelete(TRUE);
		workingHours[1]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
		workingHours[1]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
		// Tuesday
		workingHours[2] = new QPtrList<Interval>();
		workingHours[2]->setAutoDelete(TRUE);
		workingHours[2]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
		workingHours[2]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
		// Wednesday
		workingHours[3] = new QPtrList<Interval>();
		workingHours[3]->setAutoDelete(TRUE);
		workingHours[3]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
		workingHours[3]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
		// Thursday
		workingHours[4] = new QPtrList<Interval>();
		workingHours[4]->setAutoDelete(TRUE);
		workingHours[4]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
		workingHours[4]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
		// Friday
		workingHours[5] = new QPtrList<Interval>();
		workingHours[5]->setAutoDelete(TRUE);
		workingHours[5]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
		workingHours[5]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));

		// Saturday
		workingHours[6] = new QPtrList<Interval>();
	}
}

Shift::~Shift()
{
	for (int i = 0; i < 7; i++)
		delete workingHours[i];
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

