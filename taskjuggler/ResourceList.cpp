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
				   Resource* pr)
	: project(p), id(i), name(n), parent(pr)
{
	vacations.setAutoDelete(TRUE);
	jobs.setAutoDelete(TRUE);
	planJobs.setAutoDelete(TRUE);
	actualJobs.setAutoDelete(TRUE);

	if (pr)
	{
		// Inherit flags from parent resource.
		for (QStringList::Iterator it = ((FlagList*) pr)->begin();
			 it != ((FlagList*) pr)->end(); ++it)
			addFlag(*it);

		pr->subResources.append(this);

		// Inherit start values from parent resource.
		for (int i = 0; i < 7; i++)
		{
			workingHours[i] = new QPtrList<Interval>();
			workingHours[i]->setAutoDelete(TRUE);
			for (Interval* iv = pr->workingHours[i]->first(); iv != 0;
				 iv = pr->workingHours[i]->next())
				workingHours[i]->append(new Interval(*iv));
		}

		for (Interval* iv = pr->vacations.first(); iv != 0;
			 iv = pr->vacations.next())
			vacations.append(new Interval(*iv));

		minEffort = pr->minEffort;
		maxEffort = pr->maxEffort;
		rate = pr->rate;
		efficiency = pr->efficiency;
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

		if (p)
		{
			minEffort = p->getMinEffort();
			maxEffort = p->getMaxEffort();
			rate = project->getRate();
		}
		else
		{
			minEffort = 0.0;
			maxEffort = 1.0;
			rate = 0.0;
		}
		efficiency = 1.0;
	}
}

void
Resource::getSubResourceList(ResourceList& rl)
{
	for (Resource* r = subResources.first(); r != 0; r = subResources.next())
	{
		rl.append(r);
		r->getSubResourceList(rl);
	}
}

Resource*
Resource::subResourcesFirst()
{
	if (subResources.isEmpty())
	{
		currentSR = 0;
		return this;
	}

	currentSR = subResources.first();
	return currentSR->subResourcesFirst();
}

Resource*
Resource::subResourcesNext()
{
	if (currentSR == 0)
		return 0;
	Resource* tmp = currentSR->subResourcesNext();
	if (tmp == 0)
	{
		if ((currentSR = subResources.next()) == 0)
			return 0;
		return currentSR->subResourcesFirst();
	}
	return tmp;
}

bool
Resource::isAvailable(time_t date, time_t duration, Interval& interval)
{
	// Check that the resource is not closed or on vacation
	for (Interval* i = vacations.first(); i != 0; i = vacations.next())
		if (i->overlaps(Interval(date, date + duration - 1)))
			return FALSE;

	// Iterate through all the work time intervals for the week day.
	const int dow = dayOfWeek(date);
	for (Interval* i = workingHours[dow]->first(); i != 0;
		 i = workingHours[dow]->next())
	{
		/* Construct an Interval that describes the working hours for
		 * the current day using time_t. */
		interval = Interval(addTimeToDate(date, (*i).getStart()),
							addTimeToDate(date, (*i).getEnd()));
		/* If there is an overlap between working time and the requested
		 * interval we exclude the time starting with the first busy
		 * interval in that working time. */
		if (interval.overlap(Interval(date, date + duration - 1)))
		{
			time_t bookedTime = duration;
			Interval day = Interval(midnight(date),
									sameTimeNextDay(midnight(date)) - 1);
			for (Booking* b = jobs.last();
				 b != 0 && b->getStart() >= day.getStart(); b = jobs.prev())
			{
				// Check if the interval is booked already.
				if (b->getInterval().overlaps(
					Interval(date, date + duration - 1)))
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
Resource::getPlanLoad(const Interval& period, Task* task)
{
	time_t bookedTime = 0;

	double subLoad = 0.0;
	for (Resource* r = subResources.first(); r != 0;
		 r = subResources.next())
		subLoad += r->getPlanLoad(period, task);

	for (Booking* b = planJobs.first();
		 b != 0 && b->getEnd() <= period.getEnd();
		 b = planJobs.next())
	{
		Interval i = period;
		if (i.overlap(b->getInterval()) &&
			(task == 0 || task == b->getTask()))
			bookedTime += i.getDuration();
	}

	return project->convertToDailyLoad(bookedTime) * efficiency + subLoad;
}

double
Resource::getActualLoad(const Interval& period, Task* task)
{
	time_t bookedTime = 0;

	double subLoad = 0.0;
	for (Resource* r = subResources.first(); r != 0;
		 r = subResources.next())
		subLoad += r->getActualLoad(period, task);

	for (Booking* b = actualJobs.first();
		 b != 0 && b->getEnd() <= period.getEnd();
		 b = actualJobs.next())
	{
		Interval i = period;
		if (i.overlap(b->getInterval()) &&
			(task == 0 || task == b->getTask()))
			bookedTime += i.getDuration();
	}

	return project->convertToDailyLoad(bookedTime) * efficiency + subLoad;
}

double
Resource::getPlanCosts(const Interval& period, Task* task)
{
	return getPlanLoad(period, task) * rate;
}

double
Resource::getActualCosts(const Interval& period, Task* task)
{
	return getActualLoad(period, task) * rate;
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

void
Resource::preparePlan()
{
	jobs.clear();
}

void
Resource::finishPlan()
{
	planJobs.clear();
	// Make deep copy of jobs to planJobs.
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		planJobs.append(new Booking(*b));
}

void
Resource::prepareActual()
{
	jobs.clear();
}

void
Resource::finishActual()
{
	actualJobs.clear();
	// Make deep copy of jobs to actualJobs.
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		actualJobs.append(new Booking(*b));
}

ResourceList::ResourceList()
{
	sorting = Pointer;
}

void
ResourceList::createIndex()
{
	SortCriteria savedSorting = sorting;
	sorting = ResourceTree;
	sort();
	int i = 1;
	for (Resource* r = first(); r != 0; r = next(), ++i)
		r->setIndex(i);
	sorting = savedSorting;
	sort();
}

int
ResourceList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Resource* r1 = static_cast<Resource*>(i1);
	Resource* r2 = static_cast<Resource*>(i2);

	switch (sorting)
	{
	case Pointer:
		return r1->getId().compare(r2->getId());
	case ResourceTree:
	{
		QString key1;
		r1->getFullName(key1);
		QString key2;
		r2->getFullName(key2);
		if (key1 == key2)
			return 0;
		else if (key1 < key2)
			return -1;
		return 1;
	}
	default:
		qFatal("Unknown sorting criteria!\n");
		return 0;
	}
}

Resource*
ResourceList::getResource(const QString& id)
{
	Resource key(0, id, "", 0);
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
