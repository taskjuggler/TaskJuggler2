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
#include "kotrus.h"


extern Kotrus *kotrus;

int
BookingList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Booking* b1 = static_cast<Booking*>(i1);
	Booking* b2 = static_cast<Booking*>(i2);

	return b1->getInterval().compare(b2->getInterval());
}

Resource::Resource(Project* p, const QString& i, const QString& n,
				   double mie, double mae, double r)
	: project(p), id(i), name(n), minEffort(mie), maxEffort(mae), rate(r)
{
	vacations.setAutoDelete(TRUE);

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

	efficiency = 1.0;
}
bool
Resource::isAvailable(time_t date, time_t duration, Interval& interval)
{
	// Check that the resource is not closed or on vacation
	for (Interval* i = vacations.first(); i != 0; i = vacations.next())
		if (i->overlaps(Interval(date, date + duration)))
			return FALSE;

	// Iterate through all the work time intervals for the week day.
	const int dow = dayOfWeek(date);
	for (Interval* i = workingHours[dow]->first(); i != 0;
		 i = workingHours[dow]->next())
	{
		interval = Interval(midnight(date), midnight(date));
		interval.add(*i);
		/* If there is an overlap between working time and the requested
		 * interval we exclude the time starting with the first busy
		 * interval in that working time. */
		if (interval.overlap(Interval(date, date + duration)))
		{
			time_t bookedTime = duration;
			Interval day = Interval(midnight(date),
									sameTimeNextDay(midnight(date)) - 1);
			for (Booking* b = jobs.last();
				 b != 0 && b->getStart() >= day.getStart(); b = jobs.prev())
			{
				// Check if the interval is booked already.
				if (b->getInterval().overlaps(Interval(date, date + duration)))
					return FALSE;
				// Accumulate total load for the current day.
				if (day.contains(b->getInterval()))
					bookedTime += b->getDuration();
			}
			return project->convertToDailyLoad(bookedTime) <= maxEffort;
		}
	}
	return FALSE;
}

void
Resource::book(Booking* nb)
{
	// Try first to append the booking 
	for (Booking* b = jobs.last();
		 b != 0 && b->getEnd() + 1 >= nb->getStart();
		 b = jobs.prev())
	{
		if (b->getTask() == nb->getTask() &&
			b->getProjectId() == nb->getProjectId() &&
			b->getInterval().append(nb->getInterval()))
		{
			// booking appended
			delete nb;
			return;
		}
	}
	jobs.inSort(nb);
}

double
Resource::getLoadOnDay(time_t date, Task* task)
{
	return getLoad(Interval(midnight(date), sameTimeNextDay(midnight(date))),
				   task);
}

double
Resource::getLoad(const Interval& period, Task* task)
{
	time_t bookedTime = 0;

	for (Booking* b = jobs.first();
		 b != 0 && b->getEnd() <= period.getEnd();
		 b = jobs.next())
	{
		Interval i = period;
		if (i.overlap(b->getInterval()) &&
			(task == 0 || task == b->getTask()))
			bookedTime += i.getDuration();
	}

	return project->convertToDailyLoad(bookedTime) * efficiency;
}

bool
Resource::isAssignedTo(Task* task)
{
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (task == b->getTask())
			return TRUE;
	return FALSE;
}



/* retrieve all bookings _not_ belonging to this project */
bool
Resource::dbLoadBookings( const QString& kotrusID, const QString& skipProjectID )
{
   bool result = true;

   if( ! kotrus ) return( false );

   BookingList blist = kotrus->loadBookings( kotrusID, skipProjectID );
   
   return( result );
}



/* ******************************************************************************** */


bool
Resource::hasVacationDay(time_t day)
{
	Interval fullDay(midnight(day),
					 sameTimeNextDay(midnight(day)) - 1);

	if (workingHours[dayOfWeek(day)]->isEmpty())
		return TRUE;

	for (Interval* i = vacations.first(); i != 0; i = vacations.next())
		if (i->overlaps(fullDay))
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
