/*
 * ResourceList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>

#include "ResourceList.h"
#include "Task.h"
#include "Project.h"
#include "kotrus.h"

/*
 * Calls to sbIndex are fairly expensive due to the floating point
 * division. We therefor use a buffer that stores the index of the
 * first slot of a day for each slot.
 */
static int* MidnightIndex = 0;

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

	sbSize = (p->getEnd() - p->getStart()) /
		p->getScheduleGranularity() + 1;
	planScoreboard = actualScoreboard = 0;

	if (!MidnightIndex)
	{
		/*
		 * Since we need to take daylight saving time switches into account
		 * we have to add more than 24 hours to get to the next day. The
		 * buffer must be big enough so we don't create overflows.
		 */
		uint midnightIndexSize = sbSize + 
			(ONEDAY * 2) / p->getScheduleGranularity();
		MidnightIndex = new int[midnightIndexSize];
		for (uint i = 0; i < midnightIndexSize; i++)
			MidnightIndex[i] = -1;
	}
}

Resource::~Resource()
{
	for (int i = 0; i < 7; i++)
		delete workingHours[i];

	if (planScoreboard)
	{
		for (uint i = 0; i < sbSize; i++)
			if (planScoreboard[i] > (SbBooking*) 3)
			{
				uint j;
				for (j = i + 1; j < sbSize &&
						 planScoreboard[i] == planScoreboard[j]; j++)
					;
				delete planScoreboard[i];
				i = j - 1;
			}
		delete [] planScoreboard;
	}
	if (actualScoreboard)
	{
		for (uint i = 0; i < sbSize; i++)
			if (actualScoreboard[i] > (SbBooking*) 3)
			{
				uint j;
				for (j = i + 1; j < sbSize &&
						 actualScoreboard[i] == actualScoreboard[j]; j++)
					;
				delete actualScoreboard[i];
				i = j - 1;
			}
		delete [] actualScoreboard;
	}
	delete [] MidnightIndex;
	MidnightIndex = 0;
}

void
Resource::initScoreboard()
{
	scoreboard = new SbBooking*[sbSize];

	// First mark all scoreboard slots as unavailable.
	for (uint i = 0; i < sbSize; i++)
		scoreboard[i] = (SbBooking*) 1;

	// Then change all worktime slots to 0 (available) again.
	for (time_t day = project->getStart(); day < project->getEnd();
		 day = sameTimeNextDay(day))
	{
		const int dow = dayOfWeek(day);
		QPtrList<Interval>* wHours = 0;
		for (ShiftSelection* sl = shifts.first(); sl != 0;
				sl = shifts.next())
			if (sl->getPeriod().contains(day))
			{
				wHours = sl->getShift()->getWorkingHours(dow);
				break;
			}
	
		/* If we haven't found the day in the shifts we will fallback to
		 * the standard working hours. */
		if (!wHours)
			wHours = workingHours[dow];
		
		// Iterate through all the work time intervals for the week day.
		for (Interval* i = wHours->first(); i != 0; i = wHours->next())
		{
			/* Construct an Interval that describes the working hours for
			 * the current day using time_t. */
			Interval interval = Interval(addTimeToDate(day, (*i).getStart()),
										 addTimeToDate(day, (*i).getEnd()));
			for (time_t date = interval.getStart();
				 date < interval.getEnd() && date < project->getEnd();
				 date += project->getScheduleGranularity())
				scoreboard[sbIndex(date)] = (SbBooking*) 0;
		}
	}

	// Then mark all resource specific vacation slots as such (2).
	for (Interval* i = vacations.first(); i != 0; i = vacations.next())
		for (time_t date = i->getStart() > project->getStart() ?
			 i->getStart() : project->getStart();
			 date < i->getEnd() && date < project->getEnd();
			 date += project->getScheduleGranularity())
			scoreboard[sbIndex(date)] = (SbBooking*) 2;

	// Mark all global vacation slots as such (2)
	for (Interval* i = project->getVacationListFirst(); i != 0;
		 i = project->getVacationListNext())
	{
		for (time_t date = i->getStart();
			 date < i->getEnd() &&
				 project->getStart() <= date && date < project->getEnd();
			 date += project->getScheduleGranularity())
			scoreboard[sbIndex(date)] = (SbBooking*) 2;
	}
}

uint
Resource::sbIndex(time_t date) const
{
	// Convert date to corresponding scoreboard index.
	uint sbIdx = (date - project->getStart()) /
		project->getScheduleGranularity();
	if (sbIdx < 0 || sbIdx >= sbSize)
		qFatal("Date %s is outside of the defined project timeframe "
			   "(Resource %s, Index %d)",
			   time2ISO(date).latin1(),
			   id.latin1(), sbIdx);
	return sbIdx;
}

time_t
Resource::index2start(uint idx) const
{
	return project->getStart() + idx *
		project->getScheduleGranularity();
}

time_t
Resource::index2end(uint idx) const
{
	return project->getStart() + (idx + 1) *
		project->getScheduleGranularity() - 1;
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
Resource::isAvailable(time_t date, time_t duration, int loadFactor, Task* t)
{
	// Check if the interval is booked or blocked already.
	uint sbIdx = sbIndex(date);
	if (scoreboard[sbIdx])
		return FALSE;

	time_t bookedTime = duration;
	time_t bookedTimeTask = duration;

	if (MidnightIndex[sbIdx] == -1)
		MidnightIndex[sbIdx] = sbIndex(midnight(date));
	uint sbStart = MidnightIndex[sbIdx];

	uint sbIdxEnd = sbIdx +
	   	(uint) ((ONEDAY / project->getScheduleGranularity()) * 1.5);	
	if (MidnightIndex[sbIdxEnd] == -1)
		MidnightIndex[sbIdxEnd] = sbIndex(sameTimeNextDay(midnight(date)) - 1);
	uint sbEnd = MidnightIndex[sbIdxEnd];
	
	for (uint i = sbStart; i < sbEnd; i++)
   	{
		SbBooking* b = scoreboard[i];
		if (b < (SbBooking*) 4)
			continue;

		bookedTime += duration;
		if (b->getTask() == t)
			bookedTimeTask += duration;
	}

	return project->convertToDailyLoad(bookedTime) <= maxEffort &&
		project->convertToDailyLoad(bookedTimeTask)
		<= (loadFactor / 100.0);
}

bool
Resource::book(Booking* nb)
{
	uint idx = sbIndex(nb->getStart());

	return bookSlot(idx, nb);
}

bool
Resource::bookSlot(uint idx, SbBooking* nb)
{
	// Test if the time slot is still available.
	if (scoreboard[idx] != 0)
		return FALSE;
	
	SbBooking* b;
	// Try to merge the booking with the booking in the previous slot.
	if (idx > 0 && (b = scoreboard[idx - 1]) > (SbBooking*) 3 &&
		b->getTask() == nb->getTask() &&
		b->getProjectId() == nb->getProjectId())
	{
		scoreboard[idx] = b;
		delete nb;
		return TRUE;
	}
	// Try to merge the booking with the booking in the following slot.
	if (idx < sbSize - 1 && (b = scoreboard[idx + 1]) > (SbBooking*) 3 &&
		b->getTask() == nb->getTask() &&
		b->getProjectId() == nb->getProjectId())
	{
		scoreboard[idx] = b;
		delete nb;
		return TRUE;
	}
	scoreboard[idx] = nb;
	return TRUE;
}

bool
Resource::bookInterval(Booking* nb)
{
	uint sIdx = sbIndex(nb->getStart());
	uint eIdx = sbIndex(nb->getEnd());

	for (uint i = sIdx; i <= eIdx; i++)
		if (scoreboard[i])
		{
			qWarning("Resource %s is already booked at %s",
					 id.latin1(), time2ISO(index2start(i)).latin1());
			return FALSE;
		}
	for (uint i = sIdx; i <= eIdx; i++)
		bookSlot(i, new SbBooking(*nb));

	return TRUE;
}

bool
Resource::addPlanBooking(Booking* nb)
{
	SbBooking** tmp = scoreboard;
	
	if (planScoreboard)
		scoreboard = planScoreboard;
	else
		initScoreboard();
	bool retVal = bookInterval(nb);
	// Cross register booking with task.
	if (retVal && nb->getTask())
		nb->getTask()->addPlanBookedResource(this);
	delete nb;
	planScoreboard = scoreboard;
	scoreboard = tmp;
	return retVal;
}

bool
Resource::addActualBooking(Booking* nb)
{
	SbBooking** tmp = scoreboard;
	
	if (actualScoreboard)
		scoreboard = actualScoreboard;
	else
		initScoreboard();
	bool retVal = bookInterval(nb);
	// Cross register booking with task.
	if (retVal && nb->getTask())
		nb->getTask()->addActualBookedResource(this);
	actualScoreboard = scoreboard;
	scoreboard = tmp;
	return retVal;
}

double
Resource::getPlanLoad(const Interval& period, Task* task)
{
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return 0.0;

	time_t bookedTime = 0;

	double subLoad = 0.0;
	for (Resource* r = subFirst(); r != 0; r = subNext())
		subLoad += r->getPlanLoad(iv, task);

	for (uint i = sbIndex(iv.getStart());
		 i < sbIndex(iv.getEnd()) && i < sbSize; i++)
	{
		SbBooking* b = planScoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if (task == 0 || task == b->getTask())
			bookedTime += project->getScheduleGranularity();
	}

	return project->convertToDailyLoad(bookedTime) * efficiency + subLoad;
}

double
Resource::getActualLoad(const Interval& period, Task* task)
{
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return 0.0;

	time_t bookedTime = 0;

	double subLoad = 0.0;
	for (Resource* r = subFirst(); r != 0; r = subNext())
		subLoad += r->getActualLoad(iv, task);

	if (actualScoreboard)
		for (uint i = sbIndex(iv.getStart());
			 i < sbIndex(iv.getEnd()) && i < sbSize; i++)
		{
			SbBooking* b = actualScoreboard[i];
			if (b < (SbBooking*) 4)
				continue;
			if (task == 0 || task == b->getTask())
				bookedTime += project->getScheduleGranularity();
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
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return;

	for (Resource* r = subFirst(); r != 0; r = subNext())
		r->getPlanPIDs(iv, task, pids);

	for (uint i = sbIndex(iv.getStart());
		 i < sbIndex(iv.getEnd()) && i < sbSize; i++)
	{
		SbBooking* b = planScoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if ((task == 0 || task == b->getTask()) &&
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
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return;

	for (Resource* r = subFirst(); r != 0; r = subNext())
		r->getActualPIDs(iv, task, pids);

	if (actualScoreboard)
		for (uint i = sbIndex(iv.getStart());
			 i < sbIndex(iv.getEnd()) && i < sbSize; i++)
		{
			SbBooking* b = actualScoreboard[i];
			if (b < (SbBooking*) 4)
				continue;
			if ((task == 0 || task == b->getTask()) &&
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
Resource::dbLoadBookings(const QString& kotrusID,
			 const QStringList& skipProjectIDs)
{
	BookingList blist = project->getKotrus()->loadBookings
		(kotrusID, skipProjectIDs);
	return TRUE;   
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

BookingList
Resource::getPlanJobs()
{
	BookingList bl;
	if (planScoreboard)
	{
		SbBooking* b = 0;
		uint startIdx = 0;
		for (uint i = 0; i < sbSize; i++)
			if (planScoreboard[i] != b)
			{
				if (b)
					bl.append(new Booking(Interval(index2start(startIdx),
												   index2end(i - 1)),
										  planScoreboard[startIdx]));
				if (planScoreboard[i] > (SbBooking*) 3)
				{
					b = planScoreboard[i];
					startIdx = i;
				}
				else
					b = 0;
			}
	}
	return bl;
}

BookingList
Resource::getActualJobs()
{
	BookingList bl;
	if (actualScoreboard)
	{
		SbBooking* b = 0;
		uint startIdx = 0;
		for (uint i = 0; i < sbSize; i++)
			if (actualScoreboard[i] != b)
			{
				if (b)
					bl.append(new Booking(Interval(index2start(startIdx),
												   index2end(i - 1)),
										  actualScoreboard[startIdx]));
				if (actualScoreboard[i] > (SbBooking*) 3)
				{
					b = actualScoreboard[i];
					startIdx = i;
				}
				else
					b = 0;
			}
	}

	return bl;
}

void
Resource::preparePlan()
{
	if (planScoreboard)
		scoreboard = planScoreboard;
	else
		initScoreboard();
}

void
Resource::finishPlan()
{
	planScoreboard = scoreboard;
}

void
Resource::prepareActual()
{
	if (actualScoreboard)
		scoreboard = actualScoreboard;
	else
		initScoreboard();
}

void
Resource::finishActual()
{
	actualScoreboard = scoreboard;
}


QDomElement Resource::xmlIDElement( QDomDocument& doc ) const
{
   QDomElement elem = ReportXML::createXMLElem( doc, "Resource", getName());
   elem.setAttribute( "Id", getId() );
   
   return( elem );
}


/* ================================================================================ */

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
