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
#include "Project.h"

int 
ShiftSelectionList::compareItemsLevel(ShiftSelection* s1, ShiftSelection *s2,
									  int level)
{
	if (level > 2)
		return -1;
	
	if (s1->period.compare(s2->period) == 0)
		return this->compareItemsLevel(s1, s2, level + 1);
	
	return s1->period.compare(s2->period);
}

										  
int
ShiftSelectionList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	ShiftSelection* s1 = static_cast<ShiftSelection*>(i1);
	ShiftSelection* s2 = static_cast<ShiftSelection*>(i2);

	compareItemsLevel(s1, s2, 0);
}

bool
ShiftSelectionList::isOnShift(const Interval& iv)
{
	/* Check whether any of the defined shift intervals contains the interval
	 * 'iv'. If not return TRUE. If it does, check whether the interval 'iv'
	 * lies within the specified working hours. */
	for (ShiftSelection* s = first();
		 s != 0 && iv.getStart() <= s->getPeriod().getEnd(); s = next())
		if (s->getPeriod().contains(iv))
		   return s->getShift()->isOnShift(iv);
	return TRUE;
}

bool
ShiftSelectionList::isVacationDay(time_t day)
{
	for (ShiftSelection*s = first();
		 s != 0 && day <= s->getPeriod().getEnd(); s = next())
		if (s->isVacationDay(day))
			return TRUE;
	return FALSE;
}

Shift*
ShiftList::getShift(const QString& id)
{
	for (Shift* r = first(); r != 0; r = next())
		if (r->getId() == id)
			return r;

	return 0;
}

bool
ShiftSelection::isVacationDay(time_t day)
{
	return period.contains(day) && shift->isVacationDay(day);
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
		// Inherit start values from project defaults.
		for (int i = 0; i < 7; i++)
		{
			workingHours[i] = new QPtrList<Interval>();
			workingHours[i]->setAutoDelete(TRUE);
			for (const Interval* iv = prj->getWorkingHours(i)->first(); iv != 0;
				 iv = prj->getWorkingHours(i)->next())
				workingHours[i]->append(new Interval(*iv));
		}
	}
}

Shift::~Shift()
{
	for (int i = 0; i < 7; i++)
		delete workingHours[i];
}

bool
Shift::isOnShift(const Interval& iv)
{
	int dow = dayOfWeek(iv.getStart());
	for (Interval* wh = workingHours[dow]->first(); wh != 0;
		 wh = workingHours[dow]->next())
	{
		if (wh->contains(Interval(secondsOfDay(iv.getStart()),
								  secondsOfDay(iv.getEnd()))))
			return TRUE;
	}
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

