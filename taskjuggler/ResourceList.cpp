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

int Resource::debugLevel = 0;

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
		// Inherit start values from project defaults.
		for (int i = 0; i < 7; i++)
		{
			workingHours[i] = new QPtrList<Interval>();
			workingHours[i]->setAutoDelete(TRUE);
			for (const Interval* iv = p->getWorkingHours(i)->first(); iv != 0;
				 iv = p->getWorkingHours(i)->next())
				workingHours[i]->append(new Interval(*iv));
		}

		minEffort = p->getMinEffort();
		maxEffort = p->getMaxEffort();
		rate = project->getRate();
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

	// First mark all scoreboard slots as unavailable (1).
	for (uint i = 0; i < sbSize; i++)
		scoreboard[i] = (SbBooking*) 1;

	// Then change all worktime slots to 0 (available) again.
	for (time_t day = project->getStart(); day < project->getEnd();
		 day += project->getScheduleGranularity())
	{
		if (isOnShift(Interval(day,
							   day + project->getScheduleGranularity() - 1)))
			scoreboard[sbIndex(day)] = (SbBooking*) 0;
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
	{
		if (debugLevel > 6)
			qDebug("Resource %s is busy (%d)", id.latin1(), (int)
				   scoreboard[sbIdx]);
		return FALSE;
	}

	time_t bookedTime = duration;
	time_t bookedTimeTask = duration;

	if (MidnightIndex[sbIdx] == -1)
		MidnightIndex[sbIdx] = sbIndex(midnight(date));
	uint sbStart = MidnightIndex[sbIdx];

	uint sbIdxEnd = sbStart +
	   	(uint) ((ONEDAY / project->getScheduleGranularity()) * 1.5);	
	if (MidnightIndex[sbIdxEnd] == -1)
		MidnightIndex[sbIdxEnd] = sbIndex(sameTimeNextDay(midnight(date)));
	uint sbEnd = MidnightIndex[sbIdxEnd] - 1;
	
	for (uint i = sbStart; i < sbEnd; i++)
   	{
		SbBooking* b = scoreboard[i];
		if (b < (SbBooking*) 4)
			continue;

		bookedTime += duration;
		if (b->getTask() == t)
			bookedTimeTask += duration;
	}

	double resourceLoad = project->convertToDailyLoad(bookedTime);
	double taskLoad = project->convertToDailyLoad(bookedTimeTask);
	if (debugLevel > 6)
	{
		if (resourceLoad > maxEffort)
		{
			qDebug("Resource %s overloaded (%f)", id.latin1(), resourceLoad);
			return FALSE;
		}
		if (taskLoad > (loadFactor / 100.0))
		{
			qDebug("Task overloaded (%f)", loadFactor / 100.0);
			return FALSE;
		}
	}
	return  resourceLoad <= maxEffort && taskLoad <= (loadFactor / 100.0);
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
Resource::getCurrentLoad(const Interval& period, Task* task)
{
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return 0.0;

	return efficiency * project->convertToDailyLoad
		(getCurrentLoadSub(sbIndex(iv.getStart()), sbIndex(iv.getEnd()), task) *
		 project->getScheduleGranularity());
}

long
Resource::getCurrentLoadSub(uint startIdx, uint endIdx, Task* task)
{
	long bookings = 0;

	for (Resource* r = subFirst(); r != 0; r = subNext())
		bookings += r->getCurrentLoadSub(startIdx, endIdx, task);

	for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
	{
		SbBooking* b = scoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if (task == 0 || task == b->getTask())
			bookings++;
	}

	return bookings;
}

double
Resource::getPlanLoad(const Interval& period, Task* task)
{
	// If the scoreboard has not been initialized there is no load.
	if (!planScoreboard)
		return 0.0;
	
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return 0.0;

	return efficiency * project->convertToDailyLoad
		(getPlanLoadSub(sbIndex(iv.getStart()), sbIndex(iv.getEnd()), task) *
		 project->getScheduleGranularity());
}

long
Resource::getPlanLoadSub(uint startIdx, uint endIdx, Task* task)
{
	long bookings = 0;

	for (Resource* r = subFirst(); r != 0; r = subNext())
		bookings += r->getPlanLoadSub(startIdx, endIdx, task);

	for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
	{
		SbBooking* b = planScoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if (task == 0 || task == b->getTask())
			bookings++;
	}

	return bookings;
}

double
Resource::getActualLoad(const Interval& period, Task* task)
{
	// If the scoreboard has not been initialized there is no load.
	if (!actualScoreboard)
		return 0.0;
	
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return 0.0;

	return efficiency * project->convertToDailyLoad
		(getActualLoadSub(sbIndex(iv.getStart()), sbIndex(iv.getEnd()),
						  task) * project->getScheduleGranularity());
}

long
Resource::getActualLoadSub(uint startIdx, uint endIdx, Task* task)
{
	long bookings = 0;

	for (Resource* r = subFirst(); r != 0; r = subNext())
		bookings += r->getActualLoadSub(startIdx, endIdx, task);

	for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
	{
		SbBooking* b = actualScoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if (task == 0 || task == b->getTask())
			bookings++;
	}

	return bookings;
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

bool
Resource::isPlanAllocated(const Interval& period, const QString& prjId)
{
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return FALSE;

	/* If resource is a group, check members first. */
	for (Resource* r = subFirst(); r != 0; r = subNext())
		if (r->isPlanAllocated(iv, prjId))
			return TRUE;

	for (uint i = sbIndex(iv.getStart());
		 i <= sbIndex(iv.getEnd()) && i < sbSize; i++)
	{
		SbBooking* b = planScoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if (b->getProjectId() == prjId)
			return TRUE;
	}
	return FALSE;
}

bool
Resource::isActualAllocated(const Interval& period, const QString& prjId)
{
	Interval iv(period);
	if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
		return FALSE;

	/* If resource is a group, check members first. */
	for (Resource* r = subFirst(); r != 0; r = subNext())
		if (r->isActualAllocated(iv, prjId))
			return TRUE;

	for (uint i = sbIndex(iv.getStart());
		 i <= sbIndex(iv.getEnd()) && i < sbSize; i++)
	{
		SbBooking* b = actualScoreboard[i];
		if (b < (SbBooking*) 4)
			continue;
		if (b->getProjectId() == prjId)
			return TRUE;
	}
	return FALSE;
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
		 i <= sbIndex(iv.getEnd()) && i < sbSize; i++)
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
			 i <= sbIndex(iv.getEnd()) && i < sbSize; i++)
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
	for (Interval* i = vacations.first(); i != 0; i = vacations.next())
		if (i->overlaps(fullDay))
			return TRUE;

	if (shifts.isVacationDay(day))
		return TRUE;

	if (workingHours[dayOfWeek(day)]->isEmpty())
		return TRUE;

	return FALSE;
}

bool
Resource::isOnShift(const Interval& slot)
{
	for (ShiftSelection* sl = shifts.first(); sl != 0; sl = shifts.next())
		if (sl->getPeriod().contains(slot))
			return sl->getShift()->isOnShift(slot);

	int dow = dayOfWeek(slot.getStart());
	for (Interval* i = workingHours[dow]->first(); i != 0;
		 i = workingHours[dow]->next())
		if (i->contains(Interval(secondsOfDay(slot.getStart()),
								 secondsOfDay(slot.getEnd()))))
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


/* ========================================================================= */

ResourceList::ResourceList()
{
	sorting[0] = CoreAttributesList::TreeMode;
	sorting[1] = CoreAttributesList::IdUp;
}

int
ResourceList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Resource* r1 = static_cast<Resource*>(i1);
	Resource* r2 = static_cast<Resource*>(i2);

	int res;
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
		if ((res = compareItemsLevel(r1, r2, i)) != 0)
			return res;
	return res;
}

bool
ResourceList::isSupportedSortingCriteria(CoreAttributesList::SortCriteria sc)
{
	switch (sc)
	{
	case TreeMode:
	case MinEffortUp:
	case MinEffortDown:
	case MaxEffortUp:
	case MaxEffortDown:
	case RateUp:
	case RateDown:
	case KotrusIdUp:
	case KotrusIdDown:
		return TRUE;
	default:
		return CoreAttributesList::isSupportedSortingCriteria(sc);
	}		
}

int
ResourceList::compareItemsLevel(Resource* r1, Resource* r2, int level)
{
	if (level < 0 || level >= maxSortingLevel)
		return -1;

	switch (sorting[level])
	{
	case TreeMode:
		if (level == 0)
			return compareTreeItemsT(this, r1, r2);
		else
			return r1->getSequenceNo() == r2->getSequenceNo() ? 0 :
				r1->getSequenceNo() < r2->getSequenceNo() ? -1 : 1;
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
		return CoreAttributesList::compareItemsLevel(r1, r2, level);
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
