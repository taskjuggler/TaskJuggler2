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
	: CoreAttributes(p, i, n, pr)
{
	vacations.setAutoDelete(TRUE);
	jobs.setAutoDelete(TRUE);
	planJobs.setAutoDelete(TRUE);
	actualJobs.setAutoDelete(TRUE);

	if (pr)
	{
		// Inherit flags from parent resource.
		for (QStringList::Iterator it = pr->flags.begin();
			 it != pr->flags.end(); ++it)
			addFlag(*it);

		pr->sub.append(this);

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

	long sbSize = (p->getEnd() - p->getStart()) /
		p->getScheduleGranularity() + 1;
	scoreboard = new (Booking*)[sbSize];
}

long
Resource::sbIndex(time_t date) const
{
	if (date < project->getStart() || date > project->getEnd())
		qFatal("Date out of range");
	// Convert date to corresponding scoreboard index.
	return (date - project->getStart()) / project->getScheduleGranularity();
}

void
Resource::getSubResourceList(ResourceList& rl)
{
	for (Resource* r = subFirst(); r != 0; r = subNext())
	{
		rl.append(r);
		r->getSubResourceList(rl);
	}
}

Resource*
Resource::subResourcesFirst()
{
	if (sub.isEmpty())
	{
		currentSR = 0;
		return this;
	}

	currentSR = subFirst();
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
		if ((currentSR = subNext()) == 0)
			return 0;
		return currentSR->subResourcesFirst();
	}
	return tmp;
}

bool
Resource::isAvailable(time_t date, time_t duration, Interval& interval,
					  int loadFactor, Task* t)
{
	// Check if the interval is booked already.
	if (scoreboard[sbIndex(date)])
		return FALSE;

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
			time_t bookedTimeTask = duration;

			for (long i = sbIndex(midnight(date));
				 i < sbIndex(sameTimeNextDay(midnight(date)) - 1); i++)
			{
				Booking* b = scoreboard[i];
				if (b == 0)
					continue;

				bookedTime += duration;
				if (b->getTask() == t)
					bookedTimeTask += duration;
			}
			return project->convertToDailyLoad(bookedTime) <= maxEffort &&
				project->convertToDailyLoad(bookedTimeTask)
				<= (loadFactor / 100.0);
		}
	}
	return FALSE;
}

void
Resource::book(Booking* nb)
{
	long index = sbIndex(nb->getStart());

	Booking* b;
	if (index > 0 && (b = scoreboard[index - 1]) != 0 &&
		b->getTask() == nb->getTask() &&
		b->getProjectId() == nb->getProjectId())
	{
		if (!b->getInterval().append(nb->getInterval()))
			qFatal("Cannot append time slot");
		scoreboard[index] = b;
		delete nb;
		return;
	}
	if ((b = scoreboard[index + 1]) != 0 && b->getTask() == nb->getTask() &&
		b->getProjectId() == nb->getProjectId())
	{
		if (!b->getInterval().prepend(nb->getInterval()))
			qFatal("Cannot prepend time slot");
		scoreboard[index] = b;
		delete nb;
		return;
	}
	scoreboard[index] = nb;
	jobs.append(nb);
}

double
Resource::getPlanLoad(const Interval& period, Task* task)
{
	time_t bookedTime = 0;

	double subLoad = 0.0;
	for (Resource* r = subFirst(); r != 0; r = subNext())
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
	for (Resource* r = subFirst(); r != 0; r = subNext())
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
Resource::getPlanCredits(const Interval& period, Task* task)
{
	return getPlanLoad(period, task) * rate;
}

double
Resource::getActualCredits(const Interval& period, Task* task)
{
	return getActualLoad(period, task) * rate;
}

void
Resource::getPlanPIDs(const Interval& period, Task* task, QStringList& pids)
{
	for (Resource* r = subFirst(); r != 0; r = subNext())
		r->getPlanPIDs(period, task, pids);

	for (Booking* b = planJobs.first();
		 b != 0 && b->getEnd() <= period.getEnd();
		 b = planJobs.next())
	{
		Interval i = period;
		if (i.overlap(b->getInterval()) &&
			(task == 0 || task == b->getTask()) &&
			pids.findIndex(b->getProjectId()) == -1)
		{
			pids.append(b->getProjectId());
		}
	}
}

QString
Resource::getPlanProjectIDs(const Interval& period, Task* task)
{
	QStringList pids;
	getPlanPIDs(period, task, pids);
	QString pidStr;
	for (QStringList::Iterator it = pids.begin(); it != pids.end(); ++it)
		pidStr += QString(it != pids.begin() ? ", " : "") +
			project->getIdIndex(*it);

	return pidStr;
}

void
Resource::getActualPIDs(const Interval& period, Task* task, QStringList& pids)
{
	for (Resource* r = subFirst(); r != 0; r = subNext())
		r->getActualPIDs(period, task, pids);

	for (Booking* b = actualJobs.first();
		 b != 0 && b->getEnd() <= period.getEnd();
		 b = actualJobs.next())
	{
		Interval i = period;
		if (i.overlap(b->getInterval()) &&
			(task == 0 || task == b->getTask()) &&
			pids.findIndex(b->getProjectId()) == -1)
		{
			pids.append(b->getProjectId());
		}
	}
}

QString
Resource::getActualProjectIDs(const Interval& period, Task* task)
{
	QStringList pids;
	getActualPIDs(period, task, pids);
	QString pidStr;
	for (QStringList::Iterator it = pids.begin(); it != pids.end(); ++it)
		pidStr += QString(it != pids.begin() ? ", " : "") +
			project->getIdIndex(*it);

	return pidStr;
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
	for (long i = 0; i < sbSize; i++)
		scoreboard[i] = (Booking*) 0;
}

void
Resource::finishPlan()
{
	planJobs.clear();
	// Make deep copy of jobs to planJobs.
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		planJobs.append(new Booking(*b));
	planJobs.sort();
}

void
Resource::prepareActual()
{
	jobs.clear();
	for (long i = 0; i < sbSize; i++)
		scoreboard[i] = (Booking*) 0;
}

void
Resource::finishActual()
{
	actualJobs.clear();
	// Make deep copy of jobs to actualJobs.
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		actualJobs.append(new Booking(*b));
	actualJobs.sort();
}

ResourceList::ResourceList()
{
	sorting = Pointer;
}

int
ResourceList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Resource* r1 = static_cast<Resource*>(i1);
	Resource* r2 = static_cast<Resource*>(i2);

	switch (sorting)
	{
	case MinEffortUp:
		return r1->minEffort == r2->minEffort ? 0 :
			r1->minEffort < r2->minEffort ? 1 : -1;
	case MinEffortDown:
		return r1->minEffort == r2->minEffort ? 0 :
			r1->minEffort < r2->minEffort ? -1 : 1;
	case MaxEffortUp:
		return r1->maxEffort == r2->maxEffort ? 0 :
			r1->maxEffort < r2->maxEffort ? 1 : -1;
	case MaxEffortDown:
		return r1->maxEffort == r2->maxEffort ? 0 :
			r1->maxEffort < r2->maxEffort ? -1 : 1;
	case RateUp:
		return r1->rate == r2->rate ? 0 : r1->rate < r2->rate ? 1 : -1;
	case RateDown:
		return r1->rate == r2->rate ? 0 : r1->rate < r2->rate ? -1 : 1;
	case KotrusIdUp:
		return r2->kotrusId.compare(r1->kotrusId);
	case KotrusIdDown:
		return r1->kotrusId.compare(r2->kotrusId);
	default:
		return CoreAttributesList::compareItems(i1, i2);
	}
}

Resource*
ResourceList::getResource(const QString& id)
{
	for (Resource* r = first(); r != 0; r = next())
		if (r->getId() == id)
			return r;

	return 0;
}
