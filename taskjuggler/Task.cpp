/*
 * task.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

/* -- DTD --
 <!-- Task element, child of projects and for subtasks -->
 <!ELEMENT Task		(Index, Name, ProjectID, Priority, complete,
                         Type,
                         ParentTask*, Note*,
                         minStart, maxStart,
                         minEnd, maxEnd,
			 actualStart, actualEnd,
			 planStart, planEnd,
			 startBufferSize*, ActualStartBufferEnd*, PlanStartBufferEnd*,
			 endBufferSize*, ActualEndBufferStart*, PlanEndBufferStart*,
			 Resource*,
			 SubTasks*, Previous*, Follower*,
			 Allocations*, bookedResources* )>
 <!ATTLIST Task         Id CDATA #REQUIRED>
 <!ELEMENT Index        (#PCDATA)>
 <!ELEMENT Name         (#PCDATA)>
 <!ELEMENT ProjectID    (#PCDATA)>
 <!ELEMENT Priority     (#PCDATA)>
 <!ELEMENT complete     (#PCDATA)>
 <!ELEMENT Type         (#PCDATA)>
 <!ELEMENT ParentTask   (#PCDATA)>
 <!ELEMENT Note         (#PCDATA)>
 <!ELEMENT minStart     (#PCDATA)>
 <!ELEMENT maxStart     (#PCDATA)>
 <!ELEMENT minEnd       (#PCDATA)>
 <!ELEMENT maxEnd       (#PCDATA)>
 <!ELEMENT actualStart  (#PCDATA)>
 <!ELEMENT actualEnd    (#PCDATA)>
 <!ELEMENT planStart    (#PCDATA)>
 <!ELEMENT planEnd      (#PCDATA)>
 <!ELEMENT startBufferSize (#PCDATA)>
 <!ELEMENT ActualStartBufferEnd (#PCDATA)>
 <!ELEMENT PlanStartBufferEnd   (#PCDATA)>
 <!ELEMENT endBufferSize        (#PCDATA)>
 <!ELEMENT ActualEndBufferStart (#PCDATA)>
 <!ELEMENT PlanEndBufferStart    (#PCDATA)>
 <!ELEMENT SubTasks     (Task+)>
 <!ELEMENT Previous     (#PCDATA)>
 <!ELEMENT Follower     (#PCDATA)>
 <!ELEMENT bookedResources (ResourceID+)>

 <!-- Date values contain human readable date -->
 <!ATTLIST minStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST maxStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST minEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST maxEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST actualStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST actualEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST planStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST planEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST ActualStartBufferEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST PlanStartBufferEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST ActualEndBufferStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST PlanEndBufferStart
           humanReadable CDATA #REQUIRED>
   /-- DTD --/
*/
 
#include <stdio.h>
#include <stdlib.h>

#include "Task.h"
#include "Project.h"
#include "Allocation.h"

int Task::debugLevel = 0;

Task*
TaskList::getTask(const QString& id)
{
	for (Task* t = first(); t != 0; t = next())
		if (t->getId() == id)
			return t;

	return 0;
}

Task::Task(Project* proj, const QString& id_, const QString& n, Task* p,
		   const QString& f, int l)
	: CoreAttributes(proj, id_, n, p), file(f), line(l)
{
	allocations.setAutoDelete(TRUE);

	scheduling = ASAP;
	milestone = FALSE;
	complete = -1;
	startBuffer = 0.0;
	endBuffer = 0.0;
	note = "";
	account = 0;
	startCredit = endCredit = 0.0;
	lastSlot = 0;
	doneEffort = 0.0;
	doneDuration = 0.0;
	doneLength = 0.0;
	schedulingDone = FALSE;
	responsible = 0;

	if (p)
	{
		// Inherit flags from parent task.
		for (QStringList::Iterator it = p->flags.begin();
			 it != p->flags.end(); ++it)
			addFlag(*it);

		// Set attributes that are inherited from parent task.
		projectId = p->projectId;
		priority = p->priority;
		minStart = p->minStart;
		maxStart = p->maxEnd;
		minEnd = p->minStart; 
		maxEnd = p->maxEnd;
		responsible = p->responsible;
		account = p->account;
		scheduling = p->scheduling;

		// Inherit depends from parent. Relative IDs need to get another '!'.
		dependsIds = p->dependsIds;
		for (QStringList::Iterator it = dependsIds.begin();
			 it != dependsIds.end(); ++it)
		{
			if ((*it)[0] == '!')
				*it = '!' + *it;
		}
		
		// Inherit preceeds from parent. Relative IDs need to get another '!'.
		preceedsIds = p->preceedsIds;
		for (QStringList::Iterator it = preceedsIds.begin();
			 it != preceedsIds.end(); ++it)
		{
			if ((*it)[0] == '!')
				*it = '!' + *it;
		}
	}
	else
	{
		// Set attributes that are inherited from global attributes.
		projectId = proj->getCurrentId();
		priority = proj->getPriority();
		minStart = minEnd = proj->getStart();
		maxStart = maxEnd = proj->getEnd();
	}

	planStart = planEnd = 0;
	planDuration = planLength = planEffort = 0.0;

	actualStart = actualEnd = 0;
	actualDuration = actualLength = actualEffort = 0.0;

	start = end = 0;
	duration = length = effort = 0.0;
}

bool
Task::addDepends(const QString& rid)
{
   	dependsIds.append(rid);
	return TRUE;
}

bool
Task::addPreceeds(const QString& rid)
{
   	preceedsIds.append(rid);
	return TRUE;
}

void
Task::fatalError(const char* msg, ...) const
{
	va_list ap;
	va_start(ap, msg);
	char buf[1024];
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);
	
	qWarning("%s:%d:%s\n", file.latin1(), line, buf);
}

bool
Task::schedule(time_t& date, time_t slotDuration)
{
	// Task is already scheduled.
	if (schedulingDone)
	{
		qFatal("Task %s is already scheduled", id.latin1());
		return TRUE;
	}

	bool limitChanged = FALSE;
	if (start == 0 &&
		(scheduling == Task::ASAP ||
		 (length == 0.0 && duration == 0.0 && effort == 0.0 && !milestone)))
	{
		/* No start time has been specified. The start time is either
		 * start time of the parent (or the project start time if the
		 * tasks has no previous tasks) or the start time is
		 * determined by the end date of the last previous task. */
		time_t es;
		if (depends.count() == 0)
		{
			if (parent == 0)
				start = project->getStart();
			else if (getParent()->start != 0)
				start = getParent()->start;
			else
				return TRUE;
			propagateStart();
		}
		else if ((es = earliestStart()) > 0)
		{
			start = es;
			propagateStart();
		}
		else
			return TRUE;	// Task cannot be scheduled yet.

		limitChanged = TRUE;
	}

	if (end == 0 &&
		(scheduling == Task::ALAP ||
		 (length == 0.0 && duration == 0.0 && effort == 0.0 && !milestone)))
	{	
		/* No end time has been specified. The end time is either end
		 * time of the parent (or the project end time if the tasks
		 * has no previous tasks) or the end time is determined by the
		 * start date of the earliest following task. */
		time_t le;
		if (preceeds.count() == 0)
		{
			if (parent == 0)
				end = project->getEnd();
			else if (getParent()->end != 0)
				end = getParent()->end;
			else
				return TRUE;
			propagateEnd();
		}
		else if ((le = latestEnd()) > 0)
		{
			end = le;
			propagateEnd();
		}
		else			return TRUE;	// Task cannot be scheduled yet.
		
		limitChanged = TRUE;
	}

	if (scheduling == Task::ASAP)
	{
		if (lastSlot == 0)
		{
			lastSlot = start - 1;
			doneEffort = 0.0;
			doneDuration = 0.0;
			doneLength = 0.0;
			workStarted = FALSE;
			tentativeEnd = date + slotDuration - 1;
			if (debugLevel > 2)
				qWarning("Scheduling of %s starts at %s (%s)",
						 id.latin1(), time2tjp(lastSlot).latin1(),
						 time2tjp(date).latin1());
		}
		/* Do not schedule anything if the time slot is not directly
		 * following the time slot that was previously scheduled. */
		if (!((date - slotDuration <= lastSlot) && (lastSlot < date)))
			return !limitChanged;
		lastSlot = date + slotDuration - 1;
	}
	else
	{
		if (lastSlot == 0)
		{
			lastSlot = end + 1;
			doneEffort = 0.0;
			doneDuration = 0.0;
			doneLength = 0.0;
			workStarted = FALSE;
			tentativeStart = date;
			if (debugLevel > 2)
				qWarning("Scheduling of ALAP task %s starts at %s (%s)",
						 id.latin1(), time2tjp(lastSlot).latin1(),
						 time2tjp(date).latin1());
		}
		/* Do not schedule anything if the current time slot is not
		 * directly preceeding the previously scheduled time slot. */
		if (!((date + slotDuration <= lastSlot) &&
		   	(lastSlot < date + 2 * slotDuration)))
			return !limitChanged;
		lastSlot = date;
	}

	if (debugLevel > 3)
		qWarning("Scheduling %s at %s",
				 id.latin1(), time2tjp(date).latin1());

	if ((duration > 0.0) || (length > 0.0))
	{
		/* Length specifies the number of working days (as daily load)
		 * and duration specifies the number of calender days. */
		if (!allocations.isEmpty())
			bookResources(date, slotDuration);

		doneDuration += ((double) slotDuration) / ONEDAY;
		if (project->isWorkingDay(date))
			doneLength += ((double) slotDuration) / ONEDAY;

		if (debugLevel > 4)
			qWarning("Length: %f/%f   Duration: %f/%f",
					 doneLength, length,
					 doneDuration, duration);
		// Check whether we are done with this task.
		if ((length > 0.0 && doneLength >= length * 0.999999) ||
			(duration > 0.0 && doneDuration >= duration * 0.999999))
		{
			if (scheduling == ASAP)
			{
				if (doneEffort > 0.0)
				{
					end = tentativeEnd;
					date = end - slotDuration + 1;
				}
				else
					end = date + slotDuration - 1;
				propagateEnd();
			}
			else
			{
				if (doneEffort > 0.0)
				{
					start = tentativeStart;
					date = start;
				}
				else
					start = date;
				propagateStart();
			}
			project->removeActiveTask(this);
			return FALSE;
		}
	}
	else if (effort > 0.0)
	{
		/* The effort of the task has been specified. We have to look
		 * how much the resources can contribute over the following
		 * workings days until we have reached the specified
		 * effort. */
		bookResources(date, slotDuration);
		// Check whether we are done with this task.
		if (doneEffort >= effort)
		{
			if (scheduling == ASAP)
			{
				end = tentativeEnd;
				propagateEnd();
			}
			else
			{
				start = tentativeStart;
				propagateStart();
			}
			project->removeActiveTask(this);
			return FALSE;
		}
	}
	else if (milestone)
	{
		// Task is a milestone.
		if (scheduling == ASAP)
		{
			end = start;
			propagateEnd();
		}
		else
		{
			start = end;
			propagateStart();
		}
		project->removeActiveTask(this);
		return FALSE;
	}
	else if (start != 0 && end != 0)
	{
		// Task with start and end date but no duration criteria.
		if (!allocations.isEmpty() && !project->isVacation(date))
			bookResources(date, slotDuration);

		if ((scheduling == ASAP && (date + slotDuration) >= end) ||
			(scheduling == ALAP && date <= start))
		{
			project->removeActiveTask(this);
			return FALSE;
		}
	}

	return TRUE;
}

bool
Task::scheduleContainer(bool safeMode)
{
	if (schedulingDone)
		return TRUE;

	Task* t;
	time_t nstart = 0;
	time_t nend = 0;

	// Check that this is really a container task
	if ((t = subFirst()))
	{
		if (t->start == 0 || t->end == 0)
			return TRUE;
		nstart = t->start;
		nend = t->end;
	}
	else
		return TRUE;

	for (t = subNext() ; t != 0; t = subNext())
	{
		/* Make sure that all sub tasks have been scheduled. If not we
		 * can't yet schedule this task. */
		if (t->start == 0 || t->end == 0)
			return TRUE;

		if (t->start < nstart)
			nstart = t->start;
		if (t->end > nend)
			nend = t->end;
	}

	if (start == 0)
	{
		start = nstart;
		propagateStart(safeMode);
	}
	if (end == 0)
	{
		end = nend;
		propagateEnd(safeMode);
	}

	schedulingDone = TRUE;

	return FALSE;
}

void
Task::propagateStart(bool safeMode)
{
	if (start == 0)
		return;

	if (debugLevel > 1)
		qWarning("PS1: Setting start of %s to %s",
				 id.latin1(), time2tjp(start).latin1());

	for (Task* t = previous.first(); t != 0; t = previous.next())
		if (t->end == 0 && t->scheduling == ALAP &&
			t->latestEnd() != 0)
		{
			t->end = t->latestEnd();
			if (debugLevel > 1)
				qWarning("PS2: Setting end of %s to %s",
						 t->id.latin1(), time2tjp(t->end).latin1());
			t->propagateEnd(safeMode);
			if (safeMode && t->isActive())
				project->addActiveTask(t);
		}

	/* Propagate start time to sub tasks which have only an implicit
	 * dependancy on the parent task. Do not touch container tasks. */
	for (Task* t = subFirst(); t != 0; t = subNext())
	{
		if (t->start == 0 && t->previous.isEmpty() &&
			t->sub.isEmpty() && t->scheduling == ASAP)
		{
			t->start = start;
			if (debugLevel > 1)
				qWarning("PS3: Setting start of %s to %s",
						 t->id.latin1(), time2tjp(t->start).latin1());	 
			if (safeMode && t->isActive())
				project->addActiveTask(t);
			t->propagateStart(safeMode);
		}
	}

	if (safeMode && parent)
		getParent()->scheduleContainer(TRUE);
}

void
Task::propagateEnd(bool safeMode)
{
	if (end == 0)
		return;

	if (debugLevel > 1)
		qWarning("PE1: Setting end of %s to %s",
				 id.latin1(), time2tjp(end).latin1());

	for (Task* t = followers.first(); t != 0; t = followers.next())
		if (t->start == 0 && t->scheduling == ASAP &&
			t->earliestStart() != 0)
		{
			t->start = t->earliestStart();
			if (debugLevel > 1)
				qWarning("PE2: Setting start of %s to %s",
						 t->id.latin1(), time2tjp(t->start).latin1());
			t->propagateStart(safeMode);
			if (safeMode && t->isActive())
				project->addActiveTask(t);
		}
	/* Propagate end time to sub tasks which have only an implicit
	 * dependancy on the parent task. Do not touch container tasks. */
	for (Task* t = subFirst(); t != 0; t = subNext())
		if (t->end == 0 && t->followers.isEmpty() &&
			t->sub.isEmpty() && t->scheduling == ALAP)
		{
			t->end = end;
			if (debugLevel > 1)
				qWarning("PE3: Setting end of %s to %s",
						 t->id.latin1(), time2tjp(t->end).latin1());
			if (safeMode && t->isActive())
				project->addActiveTask(t);
			t->propagateEnd(safeMode);
		}

	if (safeMode && parent)
		getParent()->scheduleContainer(TRUE);
}

void
Task::propagateInitialValues()
{
	if (start != 0)
		propagateStart(FALSE);
	if (end != 0)
		propagateEnd(FALSE);
	// Check if the some data of sub tasks can already be propagated.
	if (subFirst())
		scheduleContainer(TRUE);
}

bool
Task::bookResources(time_t date, time_t slotDuration)
{
	bool allocFound = FALSE;

	/* If the time slot overlaps with a specified shift interval, the
	 * time slot must also be within the specified working hours of that
	 * shift interval. */
	if (!shifts.isOnShift(Interval(date, date + slotDuration - 1)))
	{
		if (debugLevel > 6)
			qDebug("Task %s is not active at %s", id.latin1(),
				   time2tjp(date).latin1());
		return FALSE;
	}		
		
	for (Allocation* a = allocations.first();
		 a != 0 && (effort == 0.0 || doneEffort < effort);
		 a = allocations.next())
	{
		/* If a shift has been defined for a resource for this task, there
		 * must be a shift interval defined for this day and the time must
		 * lie within the working hours of that shift. */
		if (!a->isOnShift(Interval(date, date + slotDuration - 1)))
		{
			if (debugLevel > 6)
				qDebug("Allocation not on shift at %s",
					   time2tjp(date).latin1());
			continue;
		}
		/* If the allocation has be marked persistent and a resource
		 * has already been picked, try to book this resource again. If the
		 * resource is not available there will be no booking for this
		 * time slot. */
		if (a->isPersistent() && a->getLockedResource())
			bookResource(a->getLockedResource(), date, slotDuration,
						 a->getLoad());
		else
		{
			QPtrList<Resource> cl = createCandidateList(date, a);
			for (Resource* r = cl.first(); r != 0; r = cl.next())
				if (bookResource(r, date, slotDuration, a->getLoad()))
				{
					allocFound = TRUE;
					a->setLockedResource(r);
					break;
				}
		}
	}
	return allocFound;
}

bool
Task::bookResource(Resource* r, time_t date, time_t slotDuration,
				   int loadFactor)
{
	bool booked = FALSE;
	double intervalLoad = project->convertToDailyLoad(slotDuration);

	for (Resource* rit = r->subResourcesFirst(); rit != 0;
		 rit = r->subResourcesNext())
	{
		if ((*rit).isAvailable(date, slotDuration, loadFactor, this))
		{
			(*rit).book(new Booking(
				Interval(date, date + slotDuration - 1), this,
				account ? account->getKotrusId() : QString(""),
				projectId));
			addBookedResource(rit);

			/* Move the start date to make sure that there is
			 * some work going on on the start date. */
			if (!workStarted)
			{
				if (scheduling == ASAP)
					start = date;
				else if (scheduling == ALAP)
					end = date + slotDuration - 1;
				else
					qFatal("Unknown scheduling mode");
				workStarted = TRUE;
			}

			tentativeStart = date;
			tentativeEnd = date + slotDuration - 1;
			doneEffort += intervalLoad * (*rit).getEfficiency();

			if (debugLevel > 6)
				qDebug(" Booked resource %s (Effort: %f)",
					   (*rit).getId().latin1(), doneEffort);
			booked = TRUE;
		}
	}
	return booked;
}

QPtrList<Resource>
Task::createCandidateList(time_t date, Allocation* a)
{
	/* This function generates a list of resources that could be allocated to
	 * the task. The order of the list is determined by the specified
	 * selection function of the alternatives list. From this list, the
	 * first available resource is picked later on. */
	QPtrList<Resource> candidates = a->getCandidates();
	QPtrList<Resource> cl;
	
	/* We try to minimize resource changes for consecutives time slots. So
	 * the resource used for the previous time slot is put to the 1st position
	 * of the list. */
	if (a->getLockedResource())
	{
		cl.append(a->getLockedResource());
		candidates.remove(a->getLockedResource());
		/* When an allocation is booked the resource is saved as locked
	     * resource. */
		a->setLockedResource(0);
	}
	switch (a->getSelectionMode())
	{
		case Allocation::order:
			while (candidates.first())
			{
				cl.append(candidates.first());
				candidates.remove(candidates.first());
			}
			break;
		case Allocation::minLoaded:
		{
			while (!candidates.isEmpty())
			{
				double minLoad = 0;
				Resource* minLoaded = 0;
				for (Resource* r = candidates.first(); r != 0;
					 r = candidates.next())
				{
					double load =
						r->getCurrentLoad(Interval(project->getStart(),
												   date), 0) /
						r->getMaxEffort();
					if (minLoaded == 0 || load < minLoad)
					{
						minLoad = load;
						minLoaded = r;
					}
				}
				cl.append(minLoaded);
				candidates.remove(minLoaded);
			}
			break;
		}
		case Allocation::maxLoaded:
		{
			while (!candidates.isEmpty())
			{
				double maxLoad = 0;
				Resource* maxLoaded = 0;
				for (Resource* r = candidates.first(); r != 0;
					 r = candidates.next())
				{
					double load =
						r->getCurrentLoad(Interval(project->getStart(),
												   date), 0) /
						r->getMaxEffort();
					if (maxLoaded == 0 || load > maxLoad)
					{
						maxLoad = load;
						maxLoaded = r;
					}
				}
				cl.append(maxLoaded);
				candidates.remove(maxLoaded);
			}
			break;
		}
		case Allocation::random:
		{
			while (candidates.first())
			{
				cl.append(candidates.at(rand() % candidates.count()));
				candidates.remove(candidates.first());
			}
			break;
		}
		default:
			qFatal("Illegal selection mode %d", a->getSelectionMode());
	}

	return cl;
}

bool
Task::needsEarlierTimeSlot(time_t date)
{
	if (scheduling == ALAP && lastSlot > 0 && !schedulingDone &&
		date > lastSlot && sub.isEmpty())
		return TRUE;
	if (scheduling == ASAP && lastSlot > 0 && !schedulingDone &&
		date > lastSlot + 1 && sub.isEmpty())
		return TRUE;

	return FALSE;
}

bool
Task::isCompleted(time_t date) const
{
	if (complete != -1)
	{
		// some completion degree was specified.
		return ((complete / 100.0) *
				(actualEnd - actualStart) + actualStart) > date;
	}
	

	return (project->getNow() > date);
}

time_t
Task::earliestStart()
{
	time_t date = 0;
	for (Task* t = depends.first(); t != 0; t = depends.next())
	{
		// All tasks this task depends on must have an end date set.
		if (t->end == 0)
			return 0;
		// Milestones are assumed to have duration 0.
		if (t->end > date)
			date = t->end + (t->milestone ? 0 : 1);
	}

	return date;
}

time_t
Task::latestEnd()
{
	time_t date = 0;
	for (Task* t = preceeds.first(); t != 0; t = preceeds.next())
	{
		// All tasks this task preceeds must have an start date set.
		if (t->start == 0)
			return 0;
		if (date == 0 || t->start < date)
			date = t->start - 1;
	}

	return date;
}

double
Task::getPlanCalcDuration() const
{
	time_t delta = planEnd - planStart;
	if (delta < ONEDAY)
		return (project->convertToDailyLoad(delta));
	else
		return (double) delta / ONEDAY;
}

double
Task::getPlanLoad(const Interval& period, Resource* resource)
{
	double load = 0.0;

	if (subFirst())
	{
		for (Task* t = subFirst(); t != 0; t = subNext())
			load += t->getPlanLoad(period, resource);
	}

	if (resource)
		load += resource->getPlanLoad(period, this);
	else
		for (Resource* r = planBookedResources.first(); r != 0;
			 r = planBookedResources.next())
			load += r->getPlanLoad(period, this);

	return load;
}

double
Task::getActualCalcDuration() const
{
	time_t delta = actualEnd - actualStart;
	if (delta < ONEDAY)
		return (project->convertToDailyLoad(delta));
	else
		return (double) delta / ONEDAY;
}

double
Task::getActualLoad(const Interval& period, Resource* resource)
{
	double load = 0.0;

	if (subFirst())
	{
		for (Task* t = subFirst(); t != 0; t = subNext())
			load += t->getActualLoad(period, resource);
	}
	
	if (resource)
		load += resource->getActualLoad(period, this);
	else
		for (Resource* r = actualBookedResources.first(); r != 0;
			 r = actualBookedResources.next())
			load += r->getActualLoad(period, this);

	return load;
}

double
Task::getPlanCredits(const Interval& period, Resource* resource,
					 bool recursive)
{
	double credits = 0.0;

	if (recursive && subFirst())
	{
		for (Task* t = subFirst(); t != 0; t = subNext())
			credits += t->getPlanCredits(period, resource, recursive);
	}

	if (resource)
		credits += resource->getPlanCredits(period, this);
	else
		for (Resource* r = planBookedResources.first(); r != 0;
			 r = planBookedResources.next())
			credits += r->getPlanCredits(period, this);

	if (period.contains(planStart))
		credits += startCredit;
	if (period.contains(planEnd))
		credits += endCredit;

	return credits;
}

double
Task::getActualCredits(const Interval& period, Resource* resource,
					   bool recursive)
{
	double credits = 0.0;

	if (recursive && subFirst())
	{
		for (Task* t = subFirst(); t != 0; t = subNext())
			credits += t->getActualCredits(period, resource, recursive);
	}

	if (resource)
		credits += resource->getActualCredits(period, this);
	else
		for (Resource* r = actualBookedResources.first(); r != 0;
			 r = actualBookedResources.next())
			credits += r->getActualCredits(period, this);

	if (period.contains(actualStart))
		credits += startCredit;
	if (period.contains(actualEnd))
		credits += endCredit;

	return credits;
}

bool
Task::xRef(QDict<Task>& hash)
{
	bool error = FALSE;

	if (debugLevel > 3)
		qDebug("Creating cross references for task %s", id.latin1());
	
	for (QStringList::Iterator it = dependsIds.begin();
		 it != dependsIds.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			fatalError(QString("Unknown dependency '") + absId + "'");
			error = TRUE;
		}
		else if (depends.find(t) != -1)
		{
			fatalError(QString("No need to specify dependency %1 multiple "
							   "times.").arg(absId));
			// Make it a warning only for the time beeing.
			// error = TRUE; 
		}
		else
		{
			depends.append(t);
			previous.append(t);
			t->followers.append(this);
			if (debugLevel > 3)
				qDebug("Registering follower %s with task %s",
					   id.latin1(), t->getId().latin1());
		}
	}

	for (QStringList::Iterator it = preceedsIds.begin();
		 it != preceedsIds.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			fatalError(QString("Unknown dependency '") + absId + "'");
			error = TRUE;
		}
		else if (preceeds.find(t) != -1)
		{
			fatalError(QString("No need to specify dependency '") + absId +
							   "' twice.");
			error = TRUE;
		}
		else
		{
			preceeds.append(t);
			followers.append(t);
			t->previous.append(this);
			if (debugLevel > 3)
				qDebug("Registering predecessor %s with task %s",
					   id.latin1(), t->getId().latin1());
		}
	}

	return !error;
}

QString
Task::resolveId(QString relId)
{
	/* Converts a relative ID to an absolute ID. Relative IDs start
	 * with a number of bangs. A set of bangs means 'Name of the n-th
	 * parent task' with n being the number of bangs. */
	if (relId[0] != '!')
		return relId;

	Task* t = this;
	unsigned int i;
	for (i = 0; i < relId.length() && relId.mid(i, 1) == "!"; ++i)
	{
		if (t == 0)
		{
			fatalError(QString("Illegal relative ID '") + relId + "'");
			return relId;
		}
		t = t->getParent();
	}
	if (t)
		return t->id + "." + relId.right(relId.length() - i);
	else
		return relId.right(relId.length() - i);
}

bool
Task::hasPlanStartDependency()
{
	/* Checks whether the task has a start specification for the plan
	 * scenario. This can be a fixed start time or a dependency on another
	 * task's end or an implicit dependency on the fixed start time of a
	 * parent task. */
	if (planStart != 0 || !depends.isEmpty())
		return TRUE;
	for (Task* p = getParent(); p; p = p->getParent())
		if (p->planStart != 0)
			return TRUE;
	return FALSE;
}

bool
Task::hasPlanEndDependency()
{
	/* Checks whether the task has an end specification for the plan
	 * scenario. This can be a fixed end time or a dependency on another
	 * task's start or an implicit dependency on the fixed end time of a
	 * parent task. */
	if (planEnd != 0 || !preceeds.isEmpty())
		return TRUE;
	for (Task* p = getParent(); p; p = p->getParent())
		if (p->planEnd != 0)
			return TRUE;
	return FALSE;
}

bool
Task::hasActualStartDependency()
{
	/* Checks whether the task has a start specification for the actual
	 * scenario. This can be a fixed plan or actual start time or a dependency
	 * on another task's end or an implicit dependency on the fixed plan or
	 * actual start time of a parent task. */
	if (planStart != 0 || actualStart != 0 || !depends.isEmpty())
		return TRUE;
	for (Task* p = getParent(); p; p = p->getParent())
		if (p->planStart != 0 || p->actualStart != 0)
			return TRUE;
	return FALSE;
}

bool
Task::hasActualEndDependency()
{
	/* Checks whether the task has an end specification for the actual
	 * scenario. This can be a fixed plan or actual end time or a dependency
	 * on another task's start or an implicit dependency on the fixed plan or
	 * actual end time of a parent task. */
	if (planEnd != 0 || actualEnd != 0 || !preceeds.isEmpty())
		return TRUE;
	for (Task* p = getParent(); p; p = p->getParent())
		if (p->planEnd != 0 || p->actualEnd != 0)
			return TRUE;
	return FALSE;
}

bool
Task::preScheduleOk()
{
	if ((planEffort > 0 || actualEffort > 0) && allocations.count() == 0)
	{
		fatalError(QString(
			"No allocations specified for effort based task %1").arg(1));
		return FALSE;
	}

	if (startBuffer + endBuffer >= 100.0)
	{
		fatalError("Start and end buffers may not overlap. So their sum must "
				   "be smaller then 100%.");
		return FALSE;
	}

	// Check plan values.
	int planDurationSpec = 0;
	if (planEffort > 0.0)
		planDurationSpec++;
	if (planLength > 0.0)
		planDurationSpec++;
	if (planDuration > 0.0)
		planDurationSpec++;
	if (planDurationSpec > 1)
	{
		fatalError(QString("Task %1 may only have one duration "
						   "criteria.").arg(id));
		return FALSE;
	}
	int actualDurationSpec = 0;
	if (actualEffort > 0.0 || planEffort > 0.0)
		actualDurationSpec++;
	if (actualLength > 0.0 || planLength > 0.0)
		actualDurationSpec++;
	if (actualDuration > 0.0 || planDuration > 0.0)
		actualDurationSpec++;


	/*
	|: fixed start or end date
	-: no fixed start or end date
	M: Milestone
	D: start or end dependency
    x->: ASAP task with duration criteria
    <-x: ALAP task with duration criteria
	-->: ASAP task without duration criteria
	<--: ALAP task without duration criteria
	*/
	if (!sub.isEmpty())
	{
		if (planDurationSpec != 0)
		{
			fatalError(QString("Container task %1 may not have a plan duration "
							   "criteria").arg(id));
			return FALSE;
		}
		if (actualDurationSpec != 0)
		{
			fatalError(QString("Container task %1 may not have an actual "
							   "duration criteria").arg(id));
			return FALSE;
		}
	}
	else if (milestone)
	{
		if (planDurationSpec != 0)
		{
			fatalError(QString("Milestone %1 may not have a plan duration "
							   "criteria").arg(id));
			return FALSE;
		}
		if (actualDurationSpec != 0)
		{
			fatalError(QString("Milestone %1 may not have an actual duration "
							   "criteria").arg(id));
			return FALSE;
		}
		/*
		|  M -   ok     |D M -   ok     - M -   err1   -D M -   ok
		|  M |   err2   |D M |   err2   - M |   ok     -D M |   ok
		|  M -D  ok     |D M -D  ok     - M -D  ok     -D M -D  ok
		|  M |D  err2   |D M |D  err2   - M |D  ok     -D M |D  ok
		*/
		/* err1: no start and end
		- M -
		*/
		if (!hasPlanStartDependency() && !hasPlanEndDependency())
		{
			fatalError(QString("Milestone %1 must have a plan start or end "
							   "specification.").arg(id));
			return FALSE;
		}
		if (!hasActualStartDependency() && !hasActualEndDependency())
		{
			fatalError(QString("Milestone %1 must have an actual start or end "
							   "specification.").arg(id));
			return FALSE;
		}
		/* err2: different start and end
		|  M |
		|  M |D
		|D M |
		|D M |D
		*/
		if (planStart != 0 && planEnd != 0 && planStart != planEnd)
		{
			fatalError(QString("Milestone %1 may not have both a plan start "
							   "and a plan end specification that do not "
							   "match.").arg(id));
			return FALSE;
		}
		if ((actualStart != 0 && actualEnd != 0 && actualStart != actualEnd) ||
			(actualStart == 0 && planStart != 0 && actualEnd != 0 &&
			 planStart != actualEnd) ||
			(actualStart != 0 && actualEnd == 0 && planEnd != 0 &&
			 actualStart != planEnd))
		{
			fatalError(QString("Milestone %1 may not have both an actual start "
							   "and actual end specification.").arg(id));
			return FALSE;
		}
		/* If either start of end of a milestone are specified as fixed date
		 * we set the scheduling mode, so that the fixed date it always taken,
		 * no matter what other dependencies are. */
		if ((planStart != 0 || actualStart != 0) && planEnd == 0)
			scheduling = ASAP;
		if (planStart == 0 && (planEnd != 0 || actualEnd != 0))
			scheduling = ALAP;
	}
	else
	{
		/*
		Error table for non-container, non-milestone tasks:
		
		| x-> -   ok      |D x-> -   ok      - x-> -   err3    -D x-> -   ok
		| x-> |   err1    |D x-> |   err1    - x-> |   err3    -D x-> |   err1
		| x-> -D  ok      |D x-> -D  ok      - x-> -D  err3    -D x-> -D  ok
		| x-> |D  err1    |D x-> |D  err1    - x-> |D  err3    -D x-> |D  err1
		| --> -   err2    |D --> -   err2    - --> -   err3    -D --> -   err2
		| --> |   ok      |D --> |   ok      - --> |   err3    -D --> |   ok
		| --> -D  ok      |D --> -D  ok      - --> -D  err3    -D --> -D  ok
		| --> |D  ok      |D --> |D  ok      - --> |D  err3    -D --> |D  ok
		| <-x -   err4    |D <-x -   err4    - <-x -   err4    -D <-x -   err4
		| <-x |   err1    |D <-x |   err1    - <-x |   ok      -D <-x |   ok
		| <-x -D  err1    |D <-x -D  err1    - <-x -D  ok      -D <-x -D  ok
		| <-x |D  err1    |D <-x |D  err1    - <-x |D  ok      -D <-x |D  ok
		| <-- -   err4    |D <-- -   err4    - <-- -   err4    -D <-- -   err4
		| <-- |   ok      |D <-- |   ok      - <-- |   err2    -D <-- |   ok
		| <-- -D  ok      |D <-- -D  ok      - <-- -D  err2    -D <-- -D  ok
		| <-- |D  ok      |D <-- |D  ok      - <-- |D  err2    -D <-- |D  ok
		*/
		/*
		err1: Overspecified (12 cases)
		|  x-> |
		|  <-x |
		|  x-> |D
		|  <-x |D
		|D x-> |
		|D <-x |
		|D <-x |D
		|D x-> |D
		-D x-> |
		-D x-> |D
		|D <-x -D
		|  <-x -D
		*/
		if (((planStart != 0 && planEnd != 0) ||
			 (hasPlanStartDependency() && planStart == 0 &&
			  planEnd != 0 && scheduling == ASAP) ||
			 (planStart != 0 && scheduling == ALAP &&
			  hasPlanEndDependency() && planEnd == 0)) &&
		   	planDurationSpec != 0)
		{
			fatalError(QString("Task %1 has a plan start, a plan end and a "
							   "plan duration specification.").arg(id));
			return FALSE;
		}	
		if (((actualStart != 0 && actualEnd != 0) ||
			 (hasActualStartDependency() &&
			  planStart == 0 && actualStart == 0 &&
			  (planEnd != 0 || actualEnd != 0) && scheduling == ASAP) ||
			 ((planStart != 0 || actualStart != 0) && scheduling == ALAP &&
			  hasActualEndDependency() && 
			  planEnd == 0 && actualEnd == 0)) &&
		   	actualDurationSpec != 0)
		{
			fatalError(QString("Task %1 has an actual start, an actual end "
							   "and an actual duration specification.")
					   .arg(id));
			return FALSE;
		}	
		/*
		err2: Underspecified (6 cases)
		|  --> -
		|D --> -
		-D --> -
		-  <-- |
		-  <-- |D
		-  <-- -D
		*/
		if ((hasPlanStartDependency() ^ hasPlanEndDependency()) &&
		   	planDurationSpec == 0)
		{
			fatalError(QString("Task %1 has only a plan start or end "
							   "specification but no plan duration.").arg(id));
			return FALSE;
		}
		if ((hasActualStartDependency() ^ hasActualEndDependency()) &&
		   	actualDurationSpec == 0)
		{
			fatalError(QString("Task %1 has only an actual start or end "
							   "specification but no actual duration.")
					   .arg(id));
			return FALSE;
		}
		/*
		err3: ASAP + Duration must have fixed start (8 cases)
		-  x-> -
		-  x-> |
		-  x-> -D
		-  x-> |D
		-  --> -
		-  --> |
		-  --> -D
		-  --> |D
		*/
		if (!hasPlanStartDependency() && scheduling == ASAP)
		{
			fatalError(QString("Task %1 needs a plan start specification to "
							   "be scheduled in ASAP mode.").arg(id));
			return FALSE;
		}
		if (!hasActualStartDependency() && scheduling == ASAP)
		{
			fatalError(QString("Task %1 needs an actual start specification "
							   "to be scheduled in ASAP mode.").arg(id));
			return FALSE;
		}
		/*
		err4: ALAP + Duration must have fixed end (8 cases)
		-  <-x -
		|  <-x -
		|D <-x -
		-D <-x -
		-  <-- -
		|  <-- -
		-D <-- -
		|D <-- -
		*/
		if (!hasPlanEndDependency() && scheduling == ALAP)
		{
			fatalError(QString("Task %1 needs a plan end specification to "
							   "be scheduled in ALAP mode.").arg(id));
			return FALSE;
		}
		if (!hasPlanEndDependency() && scheduling == ALAP)
		{
			fatalError(QString("Task %1 needs an actual end specification to "
							   "be scheduled in ALAP mode.").arg(id));
			return FALSE;
		}
	}

	double intervalLoad =
	   	project->convertToDailyLoad(project->getScheduleGranularity());

	for (Allocation* a = allocations.first(); a != 0; a = allocations.next())
	{
		if (a->getLoad() < intervalLoad * 100.0)
		{
			qWarning("Warning: Load is smaller than scheduling granularity "
					 "(Task: %s, Resource: %s). Minimal load is %.2f.",
					 id.latin1(), a->first()->getId().latin1(),
					 intervalLoad + 0.005);
			a->setLoad((int) (intervalLoad * 100.0));
		}
	}

	return TRUE;
}

bool
Task::scheduleOk()
{
	if (!sub.isEmpty())
	{
		// All sub task must fit into their parent task.
		for (Task* t = subFirst(); t != 0; t = subNext())
		{
			if (start > t->start)
			{
				fatalError("Task %s starts ealier than parent",
						   t->id.latin1());
				return FALSE;
			}
			if (end < t->end)
			{
				fatalError("Task %s ends later than parent", t->id.latin1());
				return FALSE;
			}
		}
	}

	if (start == 0)
	{
		fatalError("Task '%s' has no start time.", id.latin1());
		return FALSE;
	}
	if (minStart != 0 && start < minStart)
	{
		fatalError("Start time of task %s is too early\n"
				   "Date is:  %s\n"
				   "Limit is: %s",
				   id.latin1(),
				   time2tjp(start).latin1(),
				   time2tjp(minStart).latin1());
		return FALSE;
	}
	if (maxStart != 0 && start > maxStart)
	{
		fatalError("Start time of task %s is too late\n"
				   "Date is:  %s\n"
				   "Limit is: %s",
				   id.latin1(),
				   time2tjp(start).latin1(),
				   time2tjp(maxStart).latin1());
		return FALSE;
	}
	if (start < project->getStart() || start > project->getEnd())
	{
		fatalError("Start time %s of task %s is outside of project period",
			time2tjp(start).latin1(), id.latin1());
		return FALSE;
	}
	if (end == 0)
	{
		fatalError("Task '%s' has no end time.", id.latin1());
		return FALSE;
	}
	if (minEnd != 0 && end < minEnd)
	{
		fatalError("End time of task %s is too early\n"
				   "Date is:  %s\n"
				   "Limit is: %s",
				   id.latin1(),
				   time2tjp(end).latin1(),
				   time2tjp(minEnd).latin1());
		return FALSE;
	}
	if (maxEnd != 0 && end > maxEnd)
	{
		fatalError("End time of task %s is too late\n"
				   "Date is:  %s\n"
				   "Limit is: %s",
				   id.latin1(),
				   time2tjp(end).latin1(),
				   time2tjp(maxEnd).latin1());
		return FALSE;
	}
	if (end < project->getStart() || end > project->getEnd())
	{
		fatalError("End time %s of task %s is outside of project period",
			time2tjp(end).latin1(), id.latin1());
		return FALSE;
	}
	// Check if all previous tasks end before start of this task.
	for (Task* t = previous.first(); t != 0; t = previous.next())
		if (t->end > start)
		{
			fatalError("Impossible dependency:\n"
					   "Task %s ends at %s but needs to preceed\n"
					   "task %s which starts at %s",
					   t->id.latin1(), time2tjp(t->end).latin1(),
					   id.latin1(), time2tjp(start).latin1());
			return FALSE;
		}
	// Check if all following task start after this tasks end.
	for (Task* t = followers.first(); t != 0; t = followers.next())
		if (end > t->start)
		{
			fatalError("Impossible dependency:\n"
					   "Task %s starts at %s but needs to follow\n"
					   "task %s which ends at %s",
					   t->id.latin1(), time2tjp(t->start).latin1(),
					   id.latin1(), time2tjp(end).latin1());
			return FALSE;
		}

	if (!schedulingDone)
	{
		fatalError("Task %s has not been marked completed.\n"
				   "It is scheduled to last from %s to %s.\n"
				   "This might be a bug in the TaskJuggler scheduler.",
				   id.latin1(), time2tjp(start).latin1(),
				   time2tjp(end).latin1());
		return FALSE;
	}
	
	return TRUE;
}

bool
Task::isActive()
{
	if (schedulingDone || !sub.isEmpty())
		return FALSE;

	if ((scheduling == ASAP && start != 0) ||
		(scheduling == ALAP && end != 0))
		return TRUE;

	return FALSE;
}

bool
Task::isPlanActive(const Interval& period) const
{
	return period.overlaps(Interval(planStart, planEnd));
}

bool
Task::isActualActive(const Interval& period) const
{
	return period.overlaps(Interval(actualStart, actualEnd));
}

void
Task::getSubTaskList(TaskList& tl)
{
	for (Task* t = subFirst(); t != 0; t = subNext())
	{
		tl.append(t);
		t->getSubTaskList(tl);
	}
}

bool
Task::isSubTask(Task* tsk)
{
	for (Task* t = subFirst(); t != 0; t = subNext())
		if (t == tsk || t->isSubTask(tsk))
			return TRUE;

	return FALSE;
}

void
Task::preparePlan()
{
	start = planStart;
	end = planEnd;

	duration = planDuration;
	length = planLength;
	effort = planEffort;
	lastSlot = 0;
	schedulingDone = FALSE;
	bookedResources.clear();
	bookedResources = planBookedResources;

	if (actualStart == 0.0)
		actualStart = planStart;
	if (actualEnd == 0.0)
		actualEnd = planEnd;
	if (actualDuration == 0.0)
		actualDuration =  planDuration;
	if (actualLength == 0.0)
		actualLength = planLength;
	if (actualEffort == 0.0)
		actualEffort = planEffort;
}

void
Task::finishPlan()
{
	planStart = start;
	planEnd = end;
	planDuration = doneDuration;
	planLength = doneLength;
	planEffort = doneEffort;
	planBookedResources = bookedResources;
}

void
Task::prepareActual()
{
	start = actualStart;
	end = actualEnd;

	duration = actualDuration;
	length = actualLength;
	effort = actualEffort;
	lastSlot = 0;
	schedulingDone = FALSE;
	bookedResources.clear();
	bookedResources = actualBookedResources;
}

void
Task::finishActual()
{
	actualStart = start;
	actualEnd = end;
	actualDuration = doneDuration;
	actualLength = doneLength;
	actualEffort = doneEffort;
	actualBookedResources = bookedResources;
}

void
Task::computeBuffers()
{
	planStartBufferEnd = planStart - 1;
	planEndBufferStart = planEnd + 1;
	actualStartBufferEnd = actualStart - 1;
	actualEndBufferStart = actualEnd + 1;

	int sg = project->getScheduleGranularity();
	
	if (duration > 0.0)
	{
		if (startBuffer > 0.0)
		{
			planStartBufferEnd = planStart +
				(time_t) ((planEnd - planStart) * startBuffer / 100.0);
			actualStartBufferEnd = actualStart +
				(time_t) ((actualEnd - actualStart) * startBuffer / 100.0);
		}
		if (endBuffer > 0.0)
		{
			planEndBufferStart = planEnd -
				(time_t) ((planEnd - planStart) * endBuffer / 100.0);
			actualEndBufferStart = actualEnd -
				(time_t) ((actualEnd - actualStart) * endBuffer / 100.0);
		}
	}
	else if (length > 0.0)
	{
		double l;
		if (startBuffer > 0.0)
		{
			for (l = 0.0; planStartBufferEnd < planEnd;
				 planStartBufferEnd += sg)
			{
				if (project->isWorkingDay(planStartBufferEnd))
					l += (double) sg / ONEDAY;
				if (l >= planLength * startBuffer / 100.0)
					break;
			}
			for (l = 0.0; actualStartBufferEnd < actualEnd;
				 actualStartBufferEnd += sg)
			{
				if (project->isWorkingDay(actualStartBufferEnd))
					l += (double) sg / ONEDAY;
				if (l >= actualLength * startBuffer / 100.0)
					break;
			}
		}
		if (endBuffer > 0.0)
		{
			for (l = 0.0; planEndBufferStart > planStart;
				 planEndBufferStart -= sg)
			{
				if (project->isWorkingDay(planEndBufferStart))
					l += (double) sg / ONEDAY;
				if (l >= planLength * endBuffer / 100.0)
					break;
			}
			for (l = 0.0; actualEndBufferStart > actualStart;
				 actualEndBufferStart -= sg)
			{
				if (project->isWorkingDay(actualEndBufferStart))
					l += (double) sg / ONEDAY;
				if (l >= actualLength * endBuffer / 100.0)
					break;
			}
		}
	}
	else if (effort > 0.0)
	{
		double e;
		if (startBuffer > 0.0)
		{
			for (e = 0.0; planStartBufferEnd < planEnd; 
				 planStartBufferEnd += sg)
			{
				e += getPlanLoad(Interval(planStartBufferEnd,
										  planStartBufferEnd + sg));
				if (e >= planEffort * startBuffer / 100.0)
					break;
			}
			for (e = 0.0; actualStartBufferEnd < actualEnd; 
				 actualStartBufferEnd += sg)
			{
				e += getActualLoad(Interval(actualStartBufferEnd,
											actualStartBufferEnd + sg));
				if (e >= actualEffort * startBuffer / 100.0)
					break;
			}
		}
		if (endBuffer > 0.0)
		{
			for (e = 0.0; planEndBufferStart > planStart;
				 planEndBufferStart -= sg)
			{
				e += getPlanLoad(Interval(planEndBufferStart - sg,
										  planEndBufferStart));
				if (e >= planEffort * endBuffer / 100.0)
					break;
			}
			for (e = 0.0; actualEndBufferStart > actualStart;
				 actualEndBufferStart -= sg)
			{
				e += getActualLoad(Interval(actualEndBufferStart - sg,
											actualEndBufferStart));
				if (e >= actualEffort * endBuffer / 100.0)
					break;
			}
		}
	}
}

double Task::getCompleteAtTime(time_t timeSpot) const
{
   if( complete != -1 ) return( complete );

   time_t start = getPlanStart();
   time_t end = getPlanEnd();

   if( timeSpot > end ) return 100.0;
   if( timeSpot < start ) return 0.0;
   
   time_t interval = end - start;
   time_t done = timeSpot - start;

   return 100./interval*done;
}


QDomElement Task::xmlElement( QDomDocument& doc, bool /* absId */ )
{
   QDomElement taskElem = doc.createElement( "Task" );
   QDomElement tempElem;

   QString idStr = getId();
/*   if( !absId )
      idStr = idStr.section( '.', -1 ); */
      
   taskElem.setAttribute( "Id", idStr );

   QDomText t;
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Index", QString::number(getIndex()) ));
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Name", getName() ));
   taskElem.appendChild( ReportXML::createXMLElem( doc, "ProjectID", projectId ));
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Priority", QString::number(getPriority())));

   double cmplt = getCompleteAtTime( getProject()->getNow());
   taskElem.appendChild( ReportXML::createXMLElem( doc, "complete", QString::number(cmplt, 'f', 1) ));

   QString tType = "Milestone";
   if( !isMilestone() )
   {
      if( isContainer() )
	 tType = "Container";
      else
	 tType = "Task";
      
   }
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Type", tType  ));

   CoreAttributes *parent = getParent();
   if( parent )
      taskElem.appendChild( ReportXML::ReportXML::createXMLElem( doc, "ParentTask", parent->getId()));
	 
   if( !note.isEmpty())
      taskElem.appendChild( ReportXML::createXMLElem( doc, "Note", getNote()));
   
   tempElem = ReportXML::createXMLElem( doc, "minStart", QString::number( minStart ));
   tempElem.setAttribute( "humanReadable", time2ISO( minStart ));
   taskElem.appendChild( tempElem );
   
   tempElem = ReportXML::createXMLElem( doc, "maxStart", QString::number( maxStart ));
   tempElem.setAttribute( "humanReadable", time2ISO( maxStart ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "minEnd", QString::number( minEnd ));
   tempElem.setAttribute( "humanReadable", time2ISO( minEnd ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "maxEnd", QString::number( maxEnd ));
   tempElem.setAttribute( "humanReadable", time2ISO( maxEnd ));
   taskElem.appendChild( tempElem );
   
   tempElem = ReportXML::createXMLElem( doc, "actualStart", QString::number( actualStart ));
   tempElem.setAttribute( "humanReadable", time2ISO( actualStart ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "actualEnd", QString::number( actualEnd ));
   tempElem.setAttribute( "humanReadable", time2ISO( actualEnd ));
   taskElem.appendChild( tempElem );
   
   tempElem = ReportXML::createXMLElem( doc, "planStart", QString::number( planStart ));
   tempElem.setAttribute( "humanReadable", time2ISO( planStart ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "planEnd", QString::number( planEnd ));
   tempElem.setAttribute( "humanReadable", time2ISO( planEnd ));
   taskElem.appendChild( tempElem );

   /* Start- and Endbuffer */
   if( getStartBuffer() > 0.01 )
   {
      /* startbuffer exists */
      tempElem = ReportXML::createXMLElem( doc, "startBufferSize", QString::number( getStartBuffer()));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "ActualStartBufferEnd",
					   QString::number( getActualStartBufferEnd()));
      tempElem.setAttribute( "humanReadable", time2ISO(getActualStartBufferEnd()));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "PlanStartBufferEnd",
					   QString::number( getPlanStartBufferEnd()));
      tempElem.setAttribute( "humanReadable", time2ISO(getPlanStartBufferEnd()));
      taskElem.appendChild( tempElem );
      
   }

   if( getEndBuffer() > 0.01 )
   {
      /* startbuffer exists */
      tempElem = ReportXML::createXMLElem( doc, "EndBufferSize", QString::number( getEndBuffer()));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "ActualEndBufferStart",
					   QString::number( getActualEndBufferStart()));
      tempElem.setAttribute( "humanReadable", time2ISO(getActualEndBufferStart()));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "PlanEndBufferStart",
					   QString::number( getPlanEndBufferStart()));
      tempElem.setAttribute( "humanReadable", time2ISO(getPlanStartBufferEnd()));
      taskElem.appendChild( tempElem );
   }

   /* Responsible persons */
   if( getResponsible() )
      taskElem.appendChild( getResponsible()->xmlIDElement( doc ));      
   
   /* Now start the subtasks */
   int cnt = 0;
   QDomElement subTaskElem = doc.createElement( "SubTasks" );
   for (Task* t = subFirst(); t != 0; t = subNext())
   {
      if( t != this )
      {
	 QDomElement sTask = t->xmlElement( doc, false );
	 subTaskElem.appendChild( sTask );
	 cnt++;
      }
   }
   if( cnt > 0 )
      taskElem.appendChild( subTaskElem);

   /* list of tasks by id which are previous */
   if( previous.count() > 0 )
   {
      TaskList tl( previous );
      for (Task* t = tl.first(); t != 0; t = tl.next())
      {	
	 if( t != this )
	 {
	    taskElem.appendChild( ReportXML::createXMLElem( doc, "Previous", t->getId()));
	 }
      }
   }
   
   /* list of tasks by id which follow */
   if( followers.count() > 0 )
   {
      TaskList tl( followers );
      for (Task* t = tl.first(); t != 0; t = tl.next())
      {	
	 if( t != this )
	 {
	    taskElem.appendChild( ReportXML::createXMLElem( doc, "Follower", t->getId()));
	 }
      }
   }

   /** Allocations and Booked Resources
    *  With the following code, the task in XML contains simply a list of all Allocations
    *  wiht the ResourceID for which resource the allocation is. After that, there comes
    *  a list of all Resources, again having the Resource Id as key. That could be put
    *  in a hirarchy like
    *  <Resource Id="dev2" >Larry Bono
    *       <Income>1000</Income>
    *       <Allocation>
    *          <Load>100</Load>
    *          <Persistent>Yes</Persistent>
    *       </Allocation>
    *  </Resource>
    *
    *  But we do not ;-) to have full flexibility. 
    *  
    */
   /* Allocations */
   if( allocations.count() > 0 )
   {
      QPtrList<Allocation> al(allocations);
      for (Allocation* a = al.first(); a != 0; a = al.next())
      {
	 taskElem.appendChild( a->xmlElement( doc ));
      }
   }

   /* booked Ressources */
   if( bookedResources.count() > 0 )
   {	
      QPtrList<Resource> br(bookedResources);
      for (Resource* r = br.first(); r != 0; r = br.next())
      {
	 taskElem.appendChild( r->xmlIDElement( doc ));
      }
   }

   return( taskElem );
}

bool
TaskList::isSupportedSortingCriteria(CoreAttributesList::SortCriteria sc)
{
	switch (sc)
	{
	case TreeMode:
	case PlanStartUp:
	case PlanStartDown:
	case ActualStartUp:
	case ActualStartDown:
	case PlanEndUp:
	case PlanEndDown:
	case ActualEndUp:
	case ActualEndDown:
	case PrioUp:
	case PrioDown:
	case ResponsibleUp:
	case ResponsibleDown:
		return TRUE;
	default:
		return CoreAttributesList::isSupportedSortingCriteria(sc);
	}		
}

int
TaskList::compareItemsLevel(Task* t1, Task* t2, int level)
{
	if (level < 0 || level >= maxSortingLevel)
		return -1;

	switch (sorting[level])
	{
	case TreeMode:
		if (level == 0)
			return compareTreeItemsT(this, t1, t2);
		else
			return t1->getSequenceNo() == t2->getSequenceNo() ? 0 :
				t1->getSequenceNo() < t2->getSequenceNo() ? -1 : 1;
	case PlanStartUp:
		return t1->planStart == t2->planStart ? 0 :
			t1->planStart < t2->planStart ? -1 : 1;
	case PlanStartDown:
		return t1->planStart == t2->planStart ? 0 :
			t1->planStart > t2->planStart ? -1 : 1;
	case ActualStartUp:
		return t1->actualStart == t2->actualStart ? 0 :
			t1->actualStart < t2->actualStart ? -1 : 1;
	case ActualStartDown:
		return t1->actualStart == t2->actualStart ? 0 :
			t1->actualStart > t2->actualStart ? -1 : 1;
	case PlanEndUp:
		return t1->planEnd == t2->planEnd ? 0 :
			t1->planEnd < t2->planEnd ? -1 : 1;
	case PlanEndDown:
		return t1->planEnd == t2->planEnd ? 0 :
			t1->planEnd > t2->planEnd ? -1 : 1;
	case ActualEndUp:
		return t1->actualEnd == t2->actualEnd ? 0 :
			t1->actualEnd < t2->actualEnd ? -1 : 1;
	case ActualEndDown:
		return t1->actualEnd == t2->actualEnd ? 0 :
			t1->actualEnd > t2->actualEnd ? -1 : 1;
	case PrioUp:
		if (t1->priority == t2->priority)
			return 0;
		else
			return (t1->priority - t2->priority);
	case PrioDown:
		if (t1->priority == t2->priority)
			return 0;
		else
			return (t2->priority - t1->priority);
	case ResponsibleUp:
	{
		QString fn1;
		t1->responsible->getFullName(fn1);
		QString fn2;
		t2->responsible->getFullName(fn2);
		return - fn1.compare(fn2);
	}
	case ResponsibleDown:
	{
		QString fn1;
		t1->responsible->getFullName(fn1);
		QString fn2;
		t2->responsible->getFullName(fn2);
		return fn1.compare(fn2);
	}
	default:
		return CoreAttributesList::compareItemsLevel(t1, t2, level);
	}		
}

int
TaskList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Task* t1 = static_cast<Task*>(i1);
	Task* t2 = static_cast<Task*>(i2);

	int res;
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
		if ((res = compareItemsLevel(t1, t2, i)) != 0)
			return res;
	return res;
}

#ifdef HAVE_ICAL
#ifdef HAVE_KDE

void Task::toTodo( KCal::Todo* todo, KCal::CalendarLocal* /* cal */ )
{
   if( !todo ) return;
   QDateTime dt;

   // todo->setReadOnly( true );
   
   /* Start-Time of the task */
   dt.setTime_t( getPlanStart() );
   todo->setDtStart( dt );
   todo->setHasDueDate( true );
   
   /* Due-Time of the todo -> plan End  */
   dt.setTime_t( getPlanEnd());
   todo->setDtDue( dt );
   todo->setHasStartDate(true);

   /* Description and summary -> project ID */
   todo->setDescription( getNote() );
   todo->setSummary( getName() );
   todo->setPriority( getPriority() );
   todo->setCompleted( getComplete() );

   /* Resources */
   QPtrList<Resource> resList;
   resList = getPlanBookedResources();
   QStringList strList;
   
   Resource *res = 0;
   for ( res = resList.first(); res; res = resList.next() )
   {
      strList.append( res->getName());
   }
   todo->setResources(strList);
   
}

#endif /* HAVE_KDE */
#endif /* HAVE_ICAL */

void Task::loadFromXML( QDomElement& parent, Project *project )
{
   QDomElement elem = parent.firstChild().toElement();

   for( ; !elem.isNull(); elem = elem.nextSibling().toElement() )
   {
      // qDebug(  "**Task -elemType: " + elem.tagName() );
      QString elemTagName = elem.tagName();

      if( elemTagName == "Name" )
      {
        setName( elem.text());
      }
      else if( elemTagName == "SubTasks" )
      {                       
	 QDomElement subTaskElem = elem.firstChild().toElement();
	 for( ; !subTaskElem.isNull(); subTaskElem = subTaskElem.nextSibling().toElement() )
	 {
	    /* Recursive call for more tasks */
	    QString stId = subTaskElem.attribute("Id");
	    qDebug( "Recursing to elem " + stId );
	    Task *t = new Task( project, stId, QString(), this, QString(), 0 );
	    t->loadFromXML( subTaskElem, project );
	    addSub(t);
	    project->addTask(t);
	    qDebug( "Recursing to elem " + stId + " <FIN>");
	 }
      }
      else if( elemTagName == "Type" )
      {
	 if( elem.text() == "Milestone" )
	    setMilestone();
	 /* Container and Task are detected automatically */
      }
      else if( elemTagName == "Previous" )
      {
	 addDepends( elem.text() );
      }
      else if( elemTagName == "Follower" )
      {
	 addPreceeds( elem.text() );
      }
      else if( elemTagName == "Index" )
	 setIndex( elem.text().toUInt());
      else if( elemTagName == "Priority" )
        setPriority( elem.text().toInt() );
      else if( elemTagName == "complete" )
	 setComplete( elem.text().toInt() );

      /* time-stuff: */
      else if( elemTagName == "minStart" )
	 setMinStart( elem.text().toLong());
      else if( elemTagName == "maxStart" )
	 setMaxStart( elem.text().toLong() );
      else if( elemTagName == "minEnd" )
	 setMinEnd( elem.text().toLong() );
      else if( elemTagName == "maxEnd" )
	 setMaxEnd( elem.text().toLong() );
      else if( elemTagName == "actualStart" )
	 setActualStart( elem.text().toLong() );
      else if( elemTagName == "actualEnd" )
	 setActualEnd( elem.text().toLong() );
      else if( elemTagName == "planStart" )
	 setPlanStart( elem.text().toLong() );
      else if( elemTagName == "planEnd" )
	 setPlanEnd( elem.text().toLong() );
   }
}

