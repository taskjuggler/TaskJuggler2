/*
 * Shift.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Shift.h"
#include "Interval.h"
#include "Project.h"

Shift::Shift(Project* prj, const QString& i, const QString& n, Shift* p) :
	CoreAttributes(prj, i, n, p)
{
	if (p)
	{
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
Shift::isVacationDay(time_t day) const
{
	return workingHours[dayOfWeek(day, FALSE)]->isEmpty();
}

