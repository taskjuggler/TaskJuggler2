/*
 * task.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
 
#include <stdlib.h>

#include "Task.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "debug.h"
#include "Resource.h"
#include "Account.h"
#include "Project.h"
#include "ResourceTreeIterator.h"
#include "Allocation.h"
#include "Booking.h"

Task::Task(Project* proj, const QString& id_, const QString& n, Task* p,
		   const QString& f, int l)
	: CoreAttributes(proj, id_, n, p), file(f), line(l)
{
	allocations.setAutoDelete(TRUE);

	proj->addTask(this);

	scheduling = ASAP;
	milestone = FALSE;
	account = 0;
	lastSlot = 0;
	doneEffort = 0.0;
	doneDuration = 0.0;
	doneLength = 0.0;
	schedulingDone = FALSE;
	responsible = 0;

	scenarios = new TaskScenario[proj->getMaxScenarios()];
	for (int i = 0; i < proj->getMaxScenarios(); i++)
	{
		scenarios[i].task = this;
		scenarios[i].index = i;
	}

	scenarios[0].startBuffer = 0.0;
	scenarios[0].endBuffer = 0.0;
	scenarios[0].startCredit = 0.0;
	scenarios[0].endCredit = 0.0;
	
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
		
		// Inherit precedes from parent. Relative IDs need to get another '!'.
		precedesIds = p->precedesIds;
		for (QStringList::Iterator it = precedesIds.begin();
			 it != precedesIds.end(); ++it)
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
Task::addPrecedes(const QString& rid)
{
   	precedesIds.append(rid);
	return TRUE;
}

bool
Task::addShift(const Interval& i, Shift* s)
{
	return shifts.insert(new ShiftSelection(i, s));
}

void
Task::errorMessage(const char* msg, ...) const
{
	va_list ap;
	va_start(ap, msg);
	char buf[1024];
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);
	
	TJMH.errorMessage(buf, file, line);
}

void
Task::schedule(time_t& date, time_t slotDuration)
{
	// Has the task been scheduled already or is it a container?
	if (schedulingDone || !sub.isEmpty())
		return;

	if (DEBUGTS(15))
		qDebug("Trying to schedule %s at %s",
			   id.latin1(), time2tjp(date).latin1());

	if (scheduling == Task::ASAP)
	{
		if (start == 0)
			return;
		if (lastSlot == 0)
		{
			lastSlot = start - 1;
			doneEffort = 0.0;
			doneDuration = 0.0;
			doneLength = 0.0;
			workStarted = FALSE;
			tentativeEnd = date + slotDuration - 1;
			if (DEBUGTS(5))
				qDebug("Scheduling of %s starts at %s (%s)",
					   id.latin1(), time2tjp(lastSlot).latin1(),
					   time2tjp(date).latin1());
		}
		/* Do not schedule anything if the time slot is not directly
		 * following the time slot that was previously scheduled. */
		if (!((date - slotDuration <= lastSlot) && (lastSlot < date)))
			return;
		lastSlot = date + slotDuration - 1;
	}
	else
	{
		if (end == 0)
			return;
		if (lastSlot == 0)
		{
			lastSlot = end + 1;
			doneEffort = 0.0;
			doneDuration = 0.0;
			doneLength = 0.0;
			workStarted = FALSE;
			tentativeStart = date;
			if (DEBUGTS(5))
				qDebug("Scheduling of ALAP task %s starts at %s (%s)",
					   id.latin1(), time2tjp(lastSlot).latin1(),
					   time2tjp(date).latin1());
		}
		/* Do not schedule anything if the current time slot is not
		 * directly preceeding the previously scheduled time slot. */
		if (!((date + slotDuration <= lastSlot) &&
		   	(lastSlot < date + 2 * slotDuration)))
			return;
		lastSlot = date;
	}

	if (DEBUGTS(10))
		qDebug("Scheduling %s at %s",
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

		if (DEBUGTS(10))
			qDebug("Length: %f/%f   Duration: %f/%f",
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
			schedulingDone = TRUE;
			return;
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
			schedulingDone = TRUE;
			return;
		}
	}
	else if (milestone)
	{
		// Task is a milestone.
		if (scheduling == ASAP)
		{
			end = start - 1;
			propagateEnd();
		}
		else
		{
			start = end + 1;
			propagateStart();
		}
		return;
	}
	else if (start != 0 && end != 0)
	{
		// Task with start and end date but no duration criteria.
		if (!allocations.isEmpty() && !project->isVacation(date))
			bookResources(date, slotDuration);

		if ((scheduling == ASAP && (date + slotDuration) >= end) ||
			(scheduling == ALAP && date <= start))
		{
			schedulingDone = TRUE;
			return;
		}
	}

	return;
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

	if (start == 0 || (!depends.isEmpty() && start < nstart))
	{
		start = nstart;
		propagateStart(safeMode);
	}

	if (end == 0 || (!precedes.isEmpty() && nend < end))
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

	if (DEBUGTS(11))
		qDebug("PS1: Setting start of %s to %s",
			   id.latin1(), time2tjp(start).latin1());

	/* If one end of a milestone is fixed, then the other end can be set as
	 * well. */
	if (milestone && end == 0)
	{
		end = start - 1;
		schedulingDone = TRUE;
		propagateEnd(safeMode);
	}

	/* Set start date to all previous that have no start date yet, but are
	 * ALAP task or have no duration. */
	for (TaskListIterator tli(previous); *tli != 0; ++tli)
		if ((*tli)->end == 0 && (*tli)->latestEnd() != 0 &&
			((*tli)->scheduling == ALAP || 
			 ((*tli)->effort == 0.0 && (*tli)->length == 0.0 && 
			  (*tli)->duration == 0.0 && !(*tli)->milestone)))
		{
			(*tli)->end = (*tli)->latestEnd();
			if (DEBUGTS(11))
				qDebug("PS2: Setting end of %s to %s",
					   (*tli)->id.latin1(), time2tjp((*tli)->end).latin1());
			/* Recursively propagate the end date */
			(*tli)->propagateEnd(safeMode);
		}

	/* Propagate start time to sub tasks which have only an implicit
	 * dependancy on the parent task. Do not touch container tasks. */
	for (TaskListIterator tli(sub); *tli != 0; ++tli)
	{
		if ((*tli)->start == 0 && (*tli)->previous.isEmpty() &&
			(*tli)->sub.isEmpty() && (*tli)->scheduling == ASAP)
		{
			(*tli)->start = start;
			if (DEBUGTS(11))
				qDebug("PS3: Setting start of %s to %s",
					   (*tli)->id.latin1(), time2tjp((*tli)->start).latin1());	 
			/* Recursively propagate the start date */
			(*tli)->propagateStart(safeMode);
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

	if (DEBUGTS(11))
		qDebug("PE1: Setting end of %s to %s",
			   id.latin1(), time2tjp(end).latin1());

	/* If one end of a milestone is fixed, then the other end can be set as
	 * well. */
	if (milestone && start == 0)
	{
		start = end + 1;
		schedulingDone = TRUE;
		propagateStart(safeMode);
	}

	/* Set start date to all followers that have no start date yet, but are
	 * ASAP task or have no duration. */
	for (TaskListIterator tli(followers); *tli != 0; ++tli)
		if ((*tli)->start == 0 && (*tli)->earliestStart() != 0 && 
			((*tli)->scheduling == ASAP ||
			 ((*tli)->effort == 0.0 && (*tli)->length == 0.0 && 
			  (*tli)->duration == 0.0 && !(*tli)->milestone)))
		{
			(*tli)->start = (*tli)->earliestStart();
			if (DEBUGTS(11))
				qDebug("PE2: Setting start of %s to %s",
					   (*tli)->id.latin1(), time2tjp((*tli)->start).latin1());
			/* Recursively propagate the start date */
			(*tli)->propagateStart(safeMode);
		}
	/* Propagate end time to sub tasks which have only an implicit
	 * dependancy on the parent task. Do not touch container tasks. */
	for (TaskListIterator tli(sub); *tli != 0; ++tli)
		if ((*tli)->end == 0 && (*tli)->followers.isEmpty() &&
			(*tli)->sub.isEmpty() && (*tli)->scheduling == ALAP)
		{
			(*tli)->end = end;
			if (DEBUGTS(11))
				qDebug("PE3: Setting end of %s to %s",
					   (*tli)->id.latin1(), time2tjp((*tli)->end).latin1());
			/* Recursively propagate the end date */
			(*tli)->propagateEnd(safeMode);
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
	if (!sub.isEmpty())
		scheduleContainer(TRUE);
}

void
Task::setRunaway()
{
	schedulingDone = TRUE;
	runAway = TRUE;
}

bool
Task::isRunaway() const
{
	/* If a container task has runaway sub tasts, it is very likely that they
	 * are the culprits. So we don't report such a container task as runaway.
	 */
	for (TaskListIterator tli(sub); *tli != 0; ++tli)
		if ((*tli)->isRunaway())
			return FALSE;
	
	return runAway;
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
		if (DEBUGRS(15))
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
		 * be within the working hours of that shift. */
		if (!a->isOnShift(Interval(date, date + slotDuration - 1)))
		{
			if (DEBUGRS(15))
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

	for (ResourceTreeIterator rti(r); *rti != 0; ++rti)
	{
		if ((*rti)->isAvailable(date, slotDuration, loadFactor, this))
		{
			(*rti)->book(new Booking(Interval(date, date + slotDuration - 1), 
									 this, account ? account->getKotrusId() : 
									 QString(""), projectId));
			addBookedResource(*rti);

			/* Move the start date to make sure that there is
			 * some work going on at the start date. */
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
			doneEffort += intervalLoad * (*rti)->getEfficiency();

			if (DEBUGTS(6))
				qDebug(" Booked resource %s (Effort: %f)",
					   (*rti)->getId().latin1(), doneEffort);
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
Task::isCompleted(int sc, time_t date) const
{
	if (scenarios[sc].complete != -1)
	{
		// some completion degree has been specified.
		return ((scenarios[sc].complete / 100.0) *
				(scenarios[sc].end - scenarios[sc].start) 
				+ scenarios[sc].start) > date;
	}
	

	return (project->getNow() > date);
}

bool 
Task::isBuffer(int sc, const Interval& iv) const
{
	return iv.overlaps(Interval(scenarios[sc].start,
								scenarios[sc].startBufferEnd)) ||
		iv.overlaps(Interval(scenarios[sc].endBufferStart, 
							 scenarios[sc].end));
}

time_t
Task::earliestStart() const
{
	time_t date = 0;
	for (TaskListIterator tli(depends); *tli != 0; ++tli)
	{
		// All tasks this task depends on must have an end date set.
		if ((*tli)->end == 0)
			return 0;
		if ((*tli)->end > date)
			date = (*tli)->end;
	}
	if (date == 0)
		return 0;

	return date + 1;
}

time_t
Task::latestEnd() const
{
	time_t date = 0;
	for (TaskListIterator tli(precedes); *tli != 0; ++tli)
	{
		// All tasks this task preceeds must have a start date set.
		if ((*tli)->start == 0)
			return 0;
		if (date == 0 || (*tli)->start < date)
			date = (*tli)->start;
	}
	if (date == 0)
		return 0;

	return date - 1;
}

double 
Task::getCalcEffort(int sc) const
{
	return getLoad(sc, Interval(scenarios[sc].start, scenarios[sc].end));
}

double
Task::getCalcDuration(int sc) const
{
	time_t delta = scenarios[sc].end - scenarios[sc].start;
	if (delta < ONEDAY)
		return (project->convertToDailyLoad(delta));
	else
		return (double) delta / ONEDAY;
}

double
Task::getLoad(int sc, const Interval& period, const Resource* resource) const
{
	double load = 0.0;

	for (TaskListIterator tli(sub); *tli != 0; ++tli)
		load += (*tli)->getLoad(sc, period, resource);

	if (resource)
		load += resource->getLoad(sc, period, this);
	else
		for (ResourceListIterator rli(scenarios[sc].bookedResources); 
			 *rli != 0; ++rli)
			load += (*rli)->getLoad(sc, period, this);

	return load;
}

double
Task::getCredits(int sc, const Interval& period, const Resource* resource,
				 bool recursive) const
{
	double credits = 0.0;

	if (recursive && !sub.isEmpty())
	{
		for (TaskListIterator tli(sub); *tli != 0; ++tli)
			credits += (*tli)->getCredits(sc, period, resource, recursive);
	}

	if (resource)
		credits += resource->getCredits(sc, period, this);
	else
		for (ResourceListIterator rli(scenarios[sc].bookedResources);
			 *rli != 0; ++rli)
			credits += (*rli)->getCredits(sc, period, this);

	if (period.contains(scenarios[sc].start))
		credits += scenarios[sc].startCredit;
	if (period.contains(scenarios[sc].end))
		credits += scenarios[sc].endCredit;

	return credits;
}

bool
Task::xRef(QDict<Task>& hash)
{
	bool error = FALSE;

	if (DEBUGPF(2))
		qDebug("Creating cross references for task %s ...", id.latin1());
	
	for (QStringList::Iterator it = dependsIds.begin();
		 it != dependsIds.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			errorMessage(i18n("Unknown dependency '%1'").arg(absId));
			error = TRUE;
		}
		else if (depends.find(t) != -1)
		{
			errorMessage(i18n("No need to specify dependency %1 multiple "
							  "times.").arg(absId));
			// Make it a warning only for the time beeing.
			// error = TRUE; 
		}
		else
		{
			depends.append(t);
			previous.append(t);
			t->followers.append(this);
			if (DEBUGPF(11))
				qDebug("Registering follower %s with task %s",
					   id.latin1(), t->getId().latin1());
		}
	}

	for (QStringList::Iterator it = precedesIds.begin();
		 it != precedesIds.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			errorMessage(i18n("Unknown dependency '%1'").arg(absId));
			error = TRUE;
		}
		else if (precedes.find(t) != -1)
		{
			errorMessage(i18n("No need to specify dependency '%1'")
						 .arg(absId));
			// Make it a warning only for the time beeing.
			// error = TRUE; 
		}
		else
		{
			precedes.append(t);
			followers.append(t);
			t->previous.append(this);
			if (DEBUGPF(11))
				qDebug("Registering predecessor %s with task %s",
					   id.latin1(), t->getId().latin1());
		}
	}

	return !error;
}

void
Task::implicitXRef()
{
	/* Propagate implicit dependencies. If a task has no specified start or
	 * end date and no start or end dependencies, we check if a parent task
	 * has an explicit start or end date which can be used. */
	if (!sub.isEmpty() || milestone)
		return;

	bool planDurationSpec = scenarios[0].duration > 1 ||
		scenarios[0].length > 0 || scenarios[0].effort > 0;
	bool actualDurationSpec = scenarios[1].duration > 0 ||
		scenarios[1].length > 0 || scenarios[1].effort > 0 || planDurationSpec;

	if ((scenarios[0].start == 0 || scenarios[1].start == 0) &&
		depends.isEmpty())
		for (Task* tp = getParent(); tp; tp = tp->getParent())
		{
			if (tp->scenarios[0].start != 0 && scenarios[0].start == 0 &&
				(scheduling == ASAP || !planDurationSpec))
			{
				if (DEBUGPF(11))
					qDebug("Setting plan start of %s to %s", id.latin1(),
						   time2ISO(tp->scenarios[0].start).latin1());
				scenarios[0].start = tp->scenarios[0].start;
			}
			if (tp->scenarios[1].start != 0 && scenarios[1].start == 0 &&
				(scheduling == ASAP || !actualDurationSpec))
			{
				if (DEBUGPF(11))
					qDebug("Setting actual start of %s to %s", id.latin1(),
						   time2ISO(tp->scenarios[1].start).latin1());
				scenarios[1].start = tp->scenarios[1].start;
			}
		}
	/* And the same for end values */
	if ((scenarios[0].end == 0 || scenarios[1].end == 0) && precedes.isEmpty())
		for (Task* tp = getParent(); tp; tp = tp->getParent())
		{
			if (tp->scenarios[0].end != 0 && scenarios[0].end == 0 &&
				(scheduling == ALAP || !planDurationSpec))
			{
				if (DEBUGPF(11))
					qDebug("Setting plan end of %s to %s", id.latin1(),
						   time2ISO(tp->scenarios[0].end).latin1());
				scenarios[0].end = tp->scenarios[0].end;
			}
			if (tp->scenarios[1].end != 0 && scenarios[1].end == 0 &&
				(scheduling == ALAP || !actualDurationSpec))
			{
				if (DEBUGPF(11))
					qDebug("Setting actual end of %s to %s", id.latin1(),
						   time2ISO(tp->scenarios[1].end).latin1());
				scenarios[1].end = tp->scenarios[1].end;
			}
		}
}

bool
Task::loopDetector()
{
	/* Only check top-level tasks. All other tasks will be checked then as
	 * well. */
	if (parent)
		return FALSE;
	if (DEBUGPF(2))
		qDebug("Running loop detector for task %s", id.latin1());
	// Check ASAP tasks
	if (loopDetection(LDIList(), FALSE, LoopDetectorInfo::fromParent))
		return TRUE;
	// Check ALAP tasks
	if (loopDetection(LDIList(), TRUE, LoopDetectorInfo::fromParent))
		return TRUE;
	return FALSE;
}

bool
Task::loopDetection(LDIList list, bool atEnd, LoopDetectorInfo::FromWhere
					caller)
{
	if (DEBUGPF(10))
		qDebug("%sloopDetection at %s (%s)",
			   QString().fill(' ', list.count()).latin1(), id.latin1(),
			   atEnd ? "End" : "Start");

	LoopDetectorInfo thisTask(this, atEnd);

	/* If we find the current task (with same position) in the list, we have
	 * detected a loop. */
	LDIList::iterator it;
	if ((it = list.find(thisTask)) != list.end())
	{
		QString loopChain;
		for ( ; it != list.end(); ++it)
		{
			loopChain += QString("%1 (%2) -> ")
				.arg((*it).getTask()->getId())
				.arg((*it).getAtEnd() ? "End" : "Start");
			/*
			(*it).getTask()->fatalError("%s (%s) is part of loop",
			(*it).getTask()->getId().latin1(),
			(*it).getAtEnd() ?
			"End" : "Start");
			 */
		}
		loopChain += QString("%1 (%2)").arg(id)
			.arg(atEnd ? "End" : "Start");
		errorMessage(i18n("Dependency loop detected: %1").arg(loopChain));
		return TRUE;
	}
	list.append(thisTask);

	/* Now we have to traverse the graph in the direction of the specified
	 * dependencies. 'precedes' and 'depends' specify dependencies in the
	 * opposite direction of the flow of the tasks. So we have to make sure
	 * that we do not follow the arcs in the direction that precedes and
	 * depends points us. Parent/Child relationships also specify a
	 * dependency. The scheduling mode of the child determines the direction
	 * of the flow. With help of the 'caller' parameter we make sure that we
	 * only visit childs if we were referred to the task by a non-parent-child
	 * relationship. */
	if (!atEnd)
	{
		if (caller == LoopDetectorInfo::fromPrev ||
			caller == LoopDetectorInfo::fromParent)
			/* If we were not called from a sub task we check all sub tasks.*/
			for (TaskListIterator tli(sub); *tli != 0; ++tli)
			{
				if (DEBUGPF(15))
					qDebug("%sChecking sub task %s of %s",
						   QString().fill(' ', list.count()).latin1(),
						   (*tli)->getId().latin1(),
						   id.latin1());
				if ((*tli)->loopDetection(list, FALSE,
									 LoopDetectorInfo::fromParent))
					return TRUE;
			}

		if (scheduling == ASAP && sub.isEmpty())
		{
			/* Leaf task are followed in their scheduling direction. So we
			 * move from the task start to the task end. */
			if (DEBUGPF(15))
				qDebug("%sChecking end of task %s",
					   QString().fill(' ', list.count()).latin1(),
					   id.latin1());
			if (loopDetection(list, TRUE, LoopDetectorInfo::fromOtherEnd))
				return TRUE;
		}
		if (caller == LoopDetectorInfo::fromSub ||
			caller == LoopDetectorInfo::fromOtherEnd || 
			(caller == LoopDetectorInfo::fromPrev && scheduling == ALAP))
		{
			if (parent)
			{
				if (DEBUGPF(15))
					qDebug("%sChecking parent task of %s",
						   QString().fill(' ', list.count()).latin1(),	
						   id.latin1());		
				if (getParent()->loopDetection(list, FALSE,
											   LoopDetectorInfo::fromSub))
					return TRUE;
			}

			/* Now check all previous tasks that had explicit precedes on this
			 * task. */
			for (TaskListIterator tli(previous); *tli != 0; ++tli)
				if ((*tli)->precedes.find(this) != -1)
				{
					if (DEBUGPF(15))
						qDebug("%sChecking previous %s of task %s",
							   QString().fill(' ', list.count()).latin1(),
							   (*tli)->getId().latin1(), id.latin1());
					if((*tli)->loopDetection(list, TRUE, LoopDetectorInfo::fromSucc))
						return TRUE;
				}
		}
	}
	else
	{
		if (caller == LoopDetectorInfo::fromSucc ||
			caller == LoopDetectorInfo::fromParent)
			/* If we were not called from a sub task we check all sub tasks.*/
			for (TaskListIterator tli(sub); *tli != 0; ++tli)
			{
				if (DEBUGPF(15))
					qDebug("%sChecking sub task %s of %s",
						   QString().fill(' ', list.count()).latin1(),	
						   (*tli)->getId().latin1(), id.latin1());
				if ((*tli)->loopDetection(list, TRUE,
									 LoopDetectorInfo::fromParent))
					return TRUE;
			}

		if (scheduling == ALAP && sub.isEmpty())
		{
			/* Leaf task are followed in their scheduling direction. So we
			 * move from the task end to the task start. */
			if (DEBUGPF(15))
				qDebug("%sChecking start of task %s",
					   QString().fill(' ', list.count()).latin1(),	
					   id.latin1());
			if (loopDetection(list, FALSE, LoopDetectorInfo::fromOtherEnd))
				return TRUE;
		}
		if (caller == LoopDetectorInfo::fromOtherEnd ||
			caller == LoopDetectorInfo::fromSub ||
			(caller == LoopDetectorInfo::fromSucc && scheduling == ASAP))
		{
			if (parent)
			{
				if (DEBUGPF(15))
					qDebug("%sChecking parent task of %s",
						   QString().fill(' ', list.count()).latin1(),	
						   id.latin1());		
				if (getParent()->loopDetection(list, TRUE,
											   LoopDetectorInfo::fromSub))
					return TRUE;
			}

			/* Now check all following tasks that have explicit depends on this
			 * task. */
			for (TaskListIterator tli(followers); *tli != 0; ++tli)
				if ((*tli)->depends.find(this) != -1)
				{
					if (DEBUGPF(15))
						qDebug("%sChecking follower %s of task %s",
							   QString().fill(' ', list.count()).latin1(),	
							   (*tli)->getId().latin1(), id.latin1());
					if ((*tli)->loopDetection(list, FALSE,
											  LoopDetectorInfo::fromPrev))
						return TRUE;
				}
		}
	}

	if (DEBUGPF(10))
		qDebug("%sNo loops found in %s (%s)",
				 QString().fill(' ', list.count()).latin1(),	
				 id.latin1(), atEnd ? "End" : "Start");
	return FALSE;
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
			errorMessage(i18n("Illegal relative ID '%1'").arg(relId));
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
Task::hasStartDependency(int sc)
{
	/* Checks whether the task has a start specification for the 
	 * scenario. This can be a fixed start time or a dependency on another
	 * task's end or an implicit dependency on the fixed start time of a
	 * parent task. */
	if (scenarios[sc].start != 0 || !depends.isEmpty())
		return TRUE;
	for (Task* p = getParent(); p; p = p->getParent())
		if (p->scenarios[sc].start != 0)
			return TRUE;
	return FALSE;
}

bool
Task::hasEndDependency(int sc)
{
	/* Checks whether the task has an end specification for the 
	 * scenario. This can be a fixed end time or a dependency on another
	 * task's start or an implicit dependency on the fixed end time of a
	 * parent task. */
	if (scenarios[sc].end != 0 || !precedes.isEmpty())
		return TRUE;
	for (Task* p = getParent(); p; p = p->getParent())
		if (p->scenarios[sc].end != 0)
			return TRUE;
	return FALSE;
}

bool
Task::preScheduleOk()
{
	for (int sc = 0; sc < project->getMaxScenarios(); sc++)
	{
		if (scenarios[sc].effort > 0.0 && allocations.count() == 0)
		{
			errorMessage(i18n
						 ("No allocations specified for effort based task %1 "
						  "in %2 scenario")
						 .arg(1).arg(project->getScenarioName(sc)));
			qDebug(QString().sprintf("%f\n", scenarios[sc].effort));
			return FALSE;
		}

		if (scenarios[sc].startBuffer + scenarios[sc].endBuffer >= 100.0)
		{
			errorMessage(i18n
						 ("Start and end buffers may not overlap in %2 scenario. "
						  "So their sum must be smaller then 100%.")
						 .arg(project->getScenarioName(sc)));
			return FALSE;
		}

		// Check plan values.
		int durationSpec = 0;
		if (scenarios[sc].effort > 0.0)
			durationSpec++;
		if (scenarios[sc].length > 0.0)
			durationSpec++;
		if (scenarios[sc].duration > 0.0)
			durationSpec++;
		if (durationSpec > 1)
		{
			errorMessage(i18n("Task %1 may only have one duration "
							  "criteria in %2 scenario.").arg(id)
						 .arg(project->getScenarioName(sc)));
			return FALSE;
		}

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
			if (durationSpec != 0)
			{
				errorMessage(i18n
							 ("Container task %1 may not have a plan duration "
							  "criteria in %2 scenario").arg(id)
							 .arg(project->getScenarioName(sc)));
				return FALSE;
			}
		}
		else if (milestone)
		{
			if (durationSpec != 0)
			{
				errorMessage(i18n
							 ("Milestone %1 may not have a plan duration "
							  "criteria in %2 scenario").arg(id)
							 .arg(project->getScenarioName(sc)));
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
			if (!hasStartDependency(sc) && !hasEndDependency(sc))
			{
				errorMessage(i18n("Milestone %1 must have a start or end "
								  "specification in %2 scenario.")
							 .arg(id).arg(project->getScenarioName(sc)));
				return FALSE;
			}
			/* err2: different start and end
			|  M |
			|  M |D
			|D M |
			|D M |D
			 */
			if (scenarios[sc].start != 0 && scenarios[sc].end != 0 && 
				scenarios[sc].start != scenarios[sc].end + 1)
			{
				errorMessage(i18n
							 ("Milestone %1 may not have both a start "
							  "and an end specification that do not "
							  "match in %2 scenario.").arg(id)
							 .arg(project->getScenarioName(sc)));
				return FALSE;
			}
		}
		else
		{
			/*
			Error table for non-container, non-milestone tasks:

			| x-> -   ok     |D x-> -   ok     - x-> -   err3   -D x-> -   ok
			| x-> |   err1   |D x-> |   err1   - x-> |   err3   -D x-> |   err1
			| x-> -D  ok     |D x-> -D  ok     - x-> -D  err3   -D x-> -D  ok
			| x-> |D  err1   |D x-> |D  err1   - x-> |D  err3   -D x-> |D  err1
			| --> -   err2   |D --> -   err2   - --> -   err3   -D --> -   err2
			| --> |   ok     |D --> |   ok     - --> |   err3   -D --> |   ok
			| --> -D  ok     |D --> -D  ok     - --> -D  err3   -D --> -D  ok
			| --> |D  ok     |D --> |D  ok     - --> |D  err3   -D --> |D  ok
			| <-x -   err4   |D <-x -   err4   - <-x -   err4   -D <-x -   err4
			| <-x |   err1   |D <-x |   err1   - <-x |   ok     -D <-x |   ok
			| <-x -D  err1   |D <-x -D  err1   - <-x -D  ok     -D <-x -D  ok
			| <-x |D  err1   |D <-x |D  err1   - <-x |D  ok     -D <-x |D  ok
			| <-- -   err4   |D <-- -   err4   - <-- -   err4   -D <-- -   err4
			| <-- |   ok     |D <-- |   ok     - <-- |   err2   -D <-- |   ok
			| <-- -D  ok     |D <-- -D  ok     - <-- -D  err2   -D <-- -D  ok
			| <-- |D  ok     |D <-- |D  ok     - <-- |D  err2   -D <-- |D  ok
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
			if (((scenarios[sc].start != 0 && scenarios[sc].end != 0) ||
				 (hasStartDependency(sc) && scenarios[sc].start == 0 &&
				  scenarios[sc].end != 0 && scheduling == ASAP) ||
				 (scenarios[sc].start != 0 && scheduling == ALAP &&
				  hasEndDependency(sc) && scenarios[sc].end == 0)) &&
				durationSpec != 0)
			{
				errorMessage(i18n("Task %1 has a start, an end and a "
								  "duration specification for %2 scenario.")
							 .arg(id).arg(project->getScenarioName(sc)));
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
			if ((hasStartDependency(sc) ^ hasEndDependency(sc)) &&
				durationSpec == 0)
			{
				errorMessage(i18n
							 ("Task %1 has only a start or end specification "
							  "but no plan duration for the %2 scenario.")
							 .arg(id).arg(project->getScenarioName(sc)));
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
			if (!hasStartDependency(sc) && scheduling == ASAP)
			{
				errorMessage(i18n
							 ("Task %1 needs a start specification to be "
							  "scheduled in ASAP mode in the %2 scenario.")
							 .arg(id).arg(project->getScenarioName(sc)));
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
			if (!hasEndDependency(sc) && scheduling == ALAP)
			{
				errorMessage(i18n
							 ("Task %1 needs an end specification to be "
							  "scheduled in ALAP mode in the %2 scenario.")
							 .arg(id).arg(project->getScenarioName(sc)));
				return FALSE;
			}
		}
	}
	double intervalLoad =
		project->convertToDailyLoad(project->getScheduleGranularity());

	for (Allocation* a = allocations.first(); a != 0; a = allocations.next())
	{
		if (a->getLoad() < intervalLoad * 100.0)
		{
			qDebug("Warning: Load is smaller than scheduling granularity "
					 "(Task: %s, Resource: %s). Minimal load is %.2f.",
					 id.latin1(), a->first()->getId().latin1(),
					 intervalLoad + 0.005);
			a->setLoad((int) (intervalLoad * 100.0));
		}
	}

	return TRUE;
}

bool
Task::scheduleOk(int& errors, QString scenario) const
{
	/* It is of little use to report errors of container tasks, if any of
	 * their sub tasks has errors. */
	int currErrors = errors;
	for (TaskListIterator tli(sub); *tli != 0; ++tli)
		(*tli)->scheduleOk(errors, scenario);
	if (errors > currErrors)
	{
		if (DEBUGPS(2))
			qDebug(QString("Scheduling errors in sub tasks of %1.")
				   .arg(id));
		return FALSE;
	}

	/* Runaway errors have already been reported. Since the data of this task
	 * is very likely completely bogus, we just return FALSE. */
	if (runAway)
		return FALSE;

	/* If any of the dependant tasks is a runAway, we can safely surpress all
	 * other error messages. */
	for (TaskListIterator tli(depends); *tli != 0; ++tli)
		if ((*tli)->runAway)
			return FALSE;
	for (TaskListIterator tli(precedes); *tli != 0; ++tli)
		if ((*tli)->runAway)
			return FALSE;

	if (start == 0)
	{
		// Only report this for leaf tasks.
		if (DEBUGPS(1) || sub.isEmpty())
			errorMessage(i18n("Task '%1' has no %2 start time.")
						 .arg(id).arg(scenario.lower()));
		errors++;
		return FALSE;
	}
	if (start < minStart)
	{
		errorMessage(i18n("%1 start time of task %2 is too early\n"
						  "Date is:  %3\n"
						  "Limit is: %4")
					 .arg(scenario).arg(id).arg(time2tjp(start))
					 .arg(time2tjp(minStart)));
		errors++;
		return FALSE;
	}
	if (maxStart < start)
	{
		errorMessage(i18n("%1 start time of task %2 is too late\n"
						  "Date is:  %3\n"
						  "Limit is: %4")
					 .arg(scenario).arg(id)
					 .arg(time2tjp(start)).arg(time2tjp(maxStart)));
		errors++;
		return FALSE;
	}
	if (end == 0)
	{
		// Only report this for leaf tasks.
		if (DEBUGPS(1) || sub.isEmpty())
			errorMessage(i18n("Task '%1' has no %2 end time.")
						 .arg(id).arg(scenario.lower()));
		return FALSE;
	}
	if (end + (milestone ? 1 : 0) < minEnd)
	{
		errorMessage(i18n("%1 end time of task %2 is too early\n"
						  "Date is:  %3\n"
						  "Limit is: %4")
					 .arg(scenario).arg(id)
					 .arg(time2tjp(end + (milestone ? 1 : 0)))
					 .arg(time2tjp(minEnd)));
		errors++;
		return FALSE;
	}
	if (maxEnd < end + (milestone ? 1 : 0))
	{
		errorMessage(i18n("%1 end time of task %2 is too late\n"
						  "Date is:  %2\n"
						  "Limit is: %3")
					 .arg(scenario).arg(id)
					 .arg(time2tjp(end + (milestone ? 1 : 0)))
					 .arg(time2tjp(maxEnd)));
		errors++;
		return FALSE;
	}
	if (!sub.isEmpty())
	{
		// All sub task must fit into their parent task.
		for (TaskListIterator tli(sub); *tli != 0; ++tli)
		{
			if (start > (*tli)->start)
			{
				if (!(*tli)->runAway)
				{
					errorMessage(i18n("Task %1 has ealier %2 start than "
									  "parent")
								 .arg(id).arg(scenario.lower()));
					errors++;
				}
				return FALSE;
			}
			if (end < (*tli)->end)
			{
				if (!(*tli)->runAway)
				{
					errorMessage(i18n("Task %1 has later %2 end than parent")
								 .arg(id).arg(scenario.lower()));
					errors++;
				}
				return FALSE;
			}
		}
	}

	// Check if all previous tasks end before start of this task.
	for (TaskListIterator tli(previous); *tli != 0; ++tli)
		if ((*tli)->end > start && !(*tli)->runAway)
		{
			errorMessage(i18n("Impossible dependency:\n"
							  "Task %1 ends at %2 but needs to preceed\n"
							  "task %3 which has a %4 start time of %5")
						 .arg((*tli)->id).arg(time2tjp((*tli)->end).latin1())
						 .arg(id).arg(scenario.lower()).arg(time2tjp(start)));
			errors++;
			return FALSE;
		}
	// Check if all following task start after this tasks end.
	for (TaskListIterator tli(followers); *tli != 0; ++tli)
		if (end > (*tli)->start && !(*tli)->runAway)
		{
			errorMessage(i18n("Impossible dependency:\n"
							  "Task %1 starts at %2 but needs to follow\n"
							  "task %3 which has a %4 end time of %5")
						 .arg((*tli)->id).arg(time2tjp((*tli)->start))
						 .arg(id).arg(scenario.lower()).arg(time2tjp(end)));
			errors++;
			return FALSE;
		}

	if (!schedulingDone)
	{
		errorMessage(i18n("Task %1 has not been marked completed.\n"
						  "It is scheduled to last from %2 to %3.\n"
						  "This might be a bug in the TaskJuggler scheduler.")
					 .arg(id).arg(time2tjp(start)).arg(time2tjp(end)));
		errors++;
		return FALSE;
	}
	
	return TRUE;
}

time_t
Task::nextSlot(time_t slotDuration) const
{
	if (schedulingDone || !sub.isEmpty())
		return 0;

	if (scheduling == ASAP && start != 0)
	{
		if (effort == 0.0 && length == 0.0 && duration == 0.0 && !milestone &&
		   	end == 0)
			return 0;

		if (lastSlot == 0)
			return start;
		return lastSlot + 1;
	}
	if (scheduling == ALAP && end != 0)
	{
		if (effort == 0.0 && length == 0.0 && duration == 0.0 && !milestone &&
			start == 0)
			return 0;
		if (lastSlot == 0)
			return end - slotDuration + 1;
		return lastSlot - slotDuration;
	}

	return 0;
}

bool
Task::isActive(int sc, const Interval& period) const
{
	return period.overlaps(Interval(scenarios[sc].start,
									milestone ? scenarios[sc].start :
									scenarios[sc].end));
}

bool
Task::isSubTask(Task* tsk) const
{
	for (TaskListIterator tli(sub); *tli != 0; ++tli)
		if (*tli == tsk || (*tli)->isSubTask(tsk))
			return TRUE;

	return FALSE;
}

void
Task::overlayScenario(int sc)
{
	/* Scenario 0 is always the baseline. If another scenario does not provide
	 * a certain value, the value from the plan scenario is copied over. */
	if (scenarios[sc].start == 0.0)
		scenarios[sc].start = scenarios[0].start;
	if (scenarios[sc].end == 0.0)
		scenarios[sc].end = scenarios[0].end;
	if (scenarios[sc].duration == 0.0)
		scenarios[sc].duration =  scenarios[0].duration;
	if (scenarios[sc].length == 0.0)
		scenarios[sc].length = scenarios[0].length;
	if (scenarios[sc].effort == 0.0)
		scenarios[sc].effort = scenarios[0].effort;
	if (scenarios[sc].startBuffer < 0.0)
		scenarios[sc].startBuffer = scenarios[0].startBuffer;
	if (scenarios[sc].endBuffer < 0.0)
		scenarios[sc].endBuffer = scenarios[0].endBuffer;
	if (scenarios[sc].startCredit < 0.0)
		scenarios[sc].startCredit = scenarios[0].startCredit;
	if (scenarios[sc].endCredit < 0.0)
		scenarios[sc].endCredit = scenarios[0].endCredit;
	if (scenarios[sc].complete == -1)
		scenarios[sc].complete = scenarios[0].complete;
}

bool
Task::hasExtraValues(int sc) const
{
	return scenarios[sc].start != 0 || scenarios[sc].end != 0 ||
		scenarios[sc].length != 0 || scenarios[sc].duration != 0 ||
		scenarios[sc].effort != 0 || scenarios[sc].complete != -1 ||
		scenarios[sc].startBuffer >= 0.0 || scenarios[sc].endBuffer >= 0.0 ||
		scenarios[sc].startCredit >= 0.0 || scenarios[sc].endCredit >= 0.0;
}

void
Task::prepareScenario(int sc)
{
	start = scenarios[sc].start;
	end = scenarios[sc].end;

	duration = scenarios[sc].duration;
	length = scenarios[sc].length;
	effort = scenarios[sc].effort;
	lastSlot = 0;
	schedulingDone = scenarios[sc].scheduled;
	runAway = FALSE;
	bookedResources.clear();
	bookedResources = scenarios[sc].bookedResources;
}

void
Task::finishScenario(int sc)
{
	scenarios[sc].start = start;
	scenarios[sc].end = end;
	scenarios[sc].bookedResources = bookedResources;
	scenarios[sc].scheduled = schedulingDone;

	calcCompletionDegree(sc);
}

void
Task::computeBuffers()
{
	int sg = project->getScheduleGranularity();
	for (int sc = 0; sc < project->getMaxScenarios(); sc++)
	{
		scenarios[sc].startBufferEnd = scenarios[sc].start - 1;
		scenarios[sc].endBufferStart = scenarios[sc].end + 1;

	
		if (duration > 0.0)
		{
			if (scenarios[sc].startBuffer > 0.0)
				scenarios[sc].startBufferEnd = scenarios[sc].start +
					(time_t) ((scenarios[sc].end - scenarios[sc].start) * 
							  scenarios[sc].startBuffer / 100.0);
			if (scenarios[sc].endBuffer > 0.0)
				scenarios[sc].endBufferStart = scenarios[sc].end -
					(time_t) ((scenarios[sc].end - scenarios[sc].start) * 
							  scenarios[sc].endBuffer / 100.0);
		}
		else if (length > 0.0)
		{
			double l;
			if (scenarios[sc].startBuffer > 0.0)
			{
				for (l = 0.0; scenarios[sc].startBufferEnd < scenarios[sc].end;
					 scenarios[sc].startBufferEnd += sg)
				{
					if (project->isWorkingDay(scenarios[sc].startBufferEnd))
					l += (double) sg / ONEDAY;
					if (l >= scenarios[sc].length *
					   	scenarios[sc].startBuffer / 100.0)
						break;
				}
			}
			if (scenarios[sc].endBuffer > 0.0)
			{
				for (l = 0.0; scenarios[sc].endBufferStart > 
					 scenarios[sc].start; scenarios[sc].endBufferStart -= sg)
				{
					if (project->isWorkingDay(scenarios[sc].endBufferStart))
						l += (double) sg / ONEDAY;
					if (l >= scenarios[sc].length * 
						scenarios[sc].endBuffer / 100.0)
						break;
				}
			}
		}
		else if (effort > 0.0)
		{
			double e;
			if (scenarios[sc].startBuffer > 0.0)
			{
				for (e = 0.0; scenarios[sc].startBufferEnd < scenarios[sc].end; 
					 scenarios[sc].startBufferEnd += sg)
				{
					e += getLoad(sc, 
								 Interval(scenarios[sc].startBufferEnd,
										  scenarios[sc].startBufferEnd + sg));
					if (e >= scenarios[sc].effort * 
						scenarios[sc].startBuffer / 100.0)
						break;
				}
			}
			if (scenarios[sc].endBuffer > 0.0)
			{
				for (e = 0.0; scenarios[sc].endBufferStart > 
					 scenarios[sc].start; scenarios[sc].endBufferStart -= sg)
				{
					e += getLoad(sc, 
								 Interval(scenarios[sc].endBufferStart - sg,
										  scenarios[sc].endBufferStart));
					if (e >= scenarios[sc].effort * 
						scenarios[sc].endBuffer / 100.0)
						break;
				}
			}
		}
	}
}

void
Task::calcCompletionDegree(int sc)
{
	scenarios[sc].calcCompletionDegree(project->getNow());
}

double
Task::getCompletionDegree(int sc) const
{
	if(scenarios[sc].complete != -1)
		return(scenarios[sc].complete);

	return scenarios[sc].completionDegree;
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

   double cmplt = getCompletionDegree( Task::Plan);
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
   
   tempElem = ReportXML::createXMLElem( doc, "actualStart", QString::number( scenarios[1].start ));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[1].start ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "actualEnd", QString::number( scenarios[1].end ));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[1].end ));
   taskElem.appendChild( tempElem );
   
   tempElem = ReportXML::createXMLElem( doc, "planStart", QString::number( scenarios[0].start ));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[0].start ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "planEnd", QString::number( scenarios[0].end ));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[0].end ));
   taskElem.appendChild( tempElem );

   /* Start- and Endbuffer */
   if( getStartBuffer(Task::Plan) > 0.01 )
   {
      /* startbuffer exists */
      tempElem = ReportXML::createXMLElem( doc, "startBufferSize",
										   QString::number(
														   getStartBuffer(Task::Plan)));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "ActualStartBufferEnd",
					   QString::number( getStartBufferEnd(Task::Actual)));
      tempElem.setAttribute( "humanReadable",
							 time2ISO(getStartBufferEnd(Task::Actual)));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "PlanStartBufferEnd",
					   QString::number( getStartBufferEnd(Task::Plan)));
      tempElem.setAttribute( "humanReadable",
							 time2ISO(getStartBufferEnd(Task::Plan)));
      taskElem.appendChild( tempElem );
      
   }

   if( getEndBuffer(Task::Plan) > 0.01 )
   {
      /* startbuffer exists */
      tempElem = ReportXML::createXMLElem( doc, "EndBufferSize",
										   QString::number(
														   getEndBuffer(Task::Plan)));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "ActualEndBufferStart",
					   QString::number( getEndBufferStart(Task::Actual)));
      tempElem.setAttribute( "humanReadable",
							 time2ISO(getEndBufferStart(Task::Actual)));
      taskElem.appendChild( tempElem );

      tempElem = ReportXML::createXMLElem( doc, "PlanEndBufferStart",
					   QString::number( getEndBufferStart(Task::Plan)));
      tempElem.setAttribute( "humanReadable",
							 time2ISO(getStartBufferEnd(Task::Plan)));
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
	  for (TaskListIterator tli(previous); *tli != 0; ++tli)
      {	
	 if( *tli != this )
	 {
	    taskElem.appendChild( ReportXML::createXMLElem( doc, "Previous",
														(*tli)->getId()));
	 }
      }
   }
   
   /* list of tasks by id which follow */
   if( followers.count() > 0 )
   {
	  for (TaskListIterator tli(followers); *tli != 0; ++tli)
      {	
	 if( *tli != this )
	 {
	    taskElem.appendChild( ReportXML::createXMLElem( doc, "Follower",
														(*tli)->getId()));
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
	   for (ResourceListIterator rli(bookedResources); *rli != 0; ++rli)
      {
	 taskElem.appendChild( (*rli)->xmlIDElement( doc ));
      }
   }

   return( taskElem );
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
	 addPrecedes( elem.text() );
      }
      else if( elemTagName == "Index" )
	 setIndex( elem.text().toUInt());
      else if( elemTagName == "Priority" )
        setPriority( elem.text().toInt() );
      else if( elemTagName == "complete" )
	 setComplete(Task::Plan, elem.text().toInt() );

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
	 setStart(Task::Actual, elem.text().toLong() );
      else if( elemTagName == "actualEnd" )
	 setEnd(Task::Actual, elem.text().toLong() );
      else if( elemTagName == "planStart" )
	 setStart(Task::Plan, elem.text().toLong() );
      else if( elemTagName == "planEnd" )
	 setEnd(Task::Plan, elem.text().toLong() );
   }
}

