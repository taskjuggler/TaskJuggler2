/*
 * ResourceList.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>

#include "ResourceList.h"
#include "Task.h"
#include "Project.h"

Resource::Resource(Project* p, const QString& i, const QString& n,
				   double mie, double mae, double r) :
	project(p), id(i), name(n), minEffort(mie), maxEffort(mae), rate(r)
{
	// Monday
	workingHours[1].append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
	workingHours[1].append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
	// Tuesday
	workingHours[2].append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
	workingHours[2].append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
	// Wednesday
	workingHours[3].append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
	workingHours[3].append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
	// Thursday
	workingHours[4].append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
	workingHours[4].append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
	// Friday
	workingHours[5].append(new Interval(9 * ONEHOUR, 12 * ONEHOUR));
	workingHours[5].append(new Interval(13 * ONEHOUR, 18 * ONEHOUR));
}

bool
Resource::isAvailable(time_t date, time_t duration, Interval& interval)
{
	// Make sure that we don't overload the resource.
	time_t bookedTime = duration;
	Interval day = Interval(midnight(date), midnight(date) + ONEDAY - 1);
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (day.contains(b->getInterval()))
			bookedTime += b->getDuration();
	if (project->convertToDailyLoad(bookedTime) > maxEffort)
		return FALSE;

	// Iterate through all the work time intervals for the week day.
	const int dow = dayOfWeek(date);
	for (Interval* i = workingHours[dow].first(); i != 0;
		 i = workingHours[dow].next())
	{
		interval = Interval(midnight(date), midnight(date));
		interval.add(*i);
		/* If there is an overlap between working time and the requested
		 * interval we exclude the time starting with the first busy
		 * interval in that working time. */
		if (interval.overlap(Interval(date, date + duration - 1)))
		{
			for (Booking* b = jobs.first(); b != 0; b = jobs.next())
				if (!interval.exclude(b->getInterval()))
					return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

bool
Resource::book(Booking* nb)
{
	// Try first to append the booking 
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (b->getTask() == nb->getTask() &&
			b->getProjectId() == nb->getProjectId() &&
			b->getInterval().append(nb->getInterval()))
		{
			// booking appended
			delete nb;
			return TRUE;
		}
	jobs.append(nb);
	return TRUE;
}

double
Resource::getLoadOnDay(time_t date, Task* task)
{
	time_t bookedTime = 0;

	const Interval day(midnight(date), midnight(date) + ONEDAY - 1);
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
	{
		if (day.contains(b->getInterval()) &&
			(task == 0 || task == b->getTask()))
			bookedTime += b->getDuration();
	}
	return project->convertToDailyLoad(bookedTime);
}

bool
Resource::isAssignedTo(Task* task)
{
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (task == b->getTask())
			return TRUE;
	return FALSE;
}

int
ResourceList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Resource* r1 = static_cast<Resource*>(i1);
	Resource* r2 = static_cast<Resource*>(i2);

	return r1->getId().compare(r2->getId());
}

Resource*
ResourceList::getResource(const QString& id)
{
	Resource key(0, id, "");
	return at(find(&key));
}

void
Resource::printText()
{
}

void
ResourceList::printText()
{
	for (Resource* r = first(); r != 0; r = next())
		r->printText();
}


