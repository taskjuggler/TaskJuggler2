/*
 * task.cpp - TaskJuggler
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

#include "Task.h"
#include "Project.h"

Task::Task(Project* proj, const QString& id_, const QString& n, Task* p,
		   const QString f, int l)
	: project(proj), id(id_), name(n), parent(p), file(f), line(l)
{
	start = end = 0;
	actualStart = actualEnd = 0;
	length = 0;
	effort = 0.0;
	complete = -1;
	note = "";
	account = 0;
	if (p)
	{
		// Set attributes that are inherited from parent task.
		priority = p->priority;
		minStart = p->minStart;
		maxStart = p->maxStart;
		minEnd = p->minEnd;
		maxEnd = p->maxEnd;
		flags = p->flags;
		/* If parent task has attribute closed, all sub tasks will be
		 * hidden. */
		if (p->hasFlag("closed"))
			addFlag("hidden");
	}
	else
	{
		// Set attributes that are inherited from global attributes.
		priority = proj->getPriority();
		minStart = minEnd = proj->getStart();
		maxStart = maxEnd = proj->getEnd();
	}
}

void
Task::fatalError(const QString& msg) const
{
	fprintf(stderr, "%s:%d:%s\n", file.latin1(), line, msg.latin1());
}

bool
Task::schedule(time_t reqStart)
{
	// Task is already scheduled.
	if (start != 0 && end != 0)
		return TRUE;

	/* Check whether this task is a container tasks (task with sub-tasks).
	 * Container tasks are scheduled when all sub tasks have been
	 * scheduled. */
	if (!subTasks.isEmpty())
		return scheduleContainer();

	if (start == 0)
	{
		/* No start time has been specified. The start time is either the
		 * project start time if the tasks has no previous tasks, or the
		 * start time is determined by the end date of the last previous
		 * task. */
		if (previous.count() == 0 ||
			(earliestStart() > 0 && earliestStart() <= reqStart))
		{
			start = reqStart;
		}
		else
		{
			start = earliestStart();
		}
		if (start == 0)
			return FALSE;	// Task cannot be scheduled yet.
	}

	if (length > 0)
	{
		/* The task has a specified length in working days. We determine
		 * the end date by adding the specified number of working days to
		 * the start date. */
		int i = 0;
		const int oneDay = 60 * 60 * 24;
		int day;
		for (day = start; i < length; day += oneDay)
			if (isWorkingDay(day))
				++i;
		end = day;
	}
	else if (effort > 0)
	{
		/* The effort of the task has been specified. We have look how much
		 * the resources can contribute over the following workings days
		 * until we have reached the specified effort. */
		if (allocations.count() == 0)
		{
			fatalError("No allocations specified for effort based task");
			return FALSE;
		}
		double done = 0.0;
		const int oneDay = 60 * 60 * 24;
		int day;
		bool workStarted = FALSE;
		day = start;
		for ( ; ; )
		{
			double costs = 0.0;
			if (isWorkingDay(day))
			{
				if (!bookResources(day, workStarted, done, costs))
//					fprintf(stderr,
//							"No resource available for task '%s' on %s\n",
//							id.latin1(),
//							time2ISO(day).latin1())
					;
			}
			/* If an account has been specified load account with the
			 * accumulated costs of this day. */
			if (account)
				account->book(new Transaction(day, -costs, this));
			if (done < effort)
				day += oneDay;
			else
			{
				end = day;
				break;
			}
		}
	}
	else
	{
		// Task is a milestone.
		end = start;
	}
	return TRUE;
}

bool
Task::scheduleContainer()
{
	Task* t;
	time_t nstart = 0;
	time_t nend = 0;

	if ((t = subTasks.first()) && (t != 0))
	{
		if (t->getStart() > 0)
			nstart = t->getStart();
		if (t->getEnd() > 0)
			nend = t->getEnd();
	}
	else
		return TRUE;

	for (t = subTasks.next() ; t != 0; t = subTasks.next())
	{
		if (t->getStart() < start)
			nstart = t->getStart();
		if (t->getEnd() > end)
			nend = t->getEnd();
	}

	/* Make sure that all sub tasks have been scheduled. If not we can't
	 * yet schedule this task. */
	if (nstart > 0 && nend > 0)
	{
		start = nstart;
		end = nend;
		return TRUE;
	}

	return FALSE;
}

bool
Task::bookResources(time_t day, bool& workStarted, double& done, double& costs)
{
	bool allocFound = FALSE;

	for (Allocation* a = allocations.first(); a != 0;
		 a = allocations.next())
	{
		/* Move the start date to make sure that there is
		 * some work going on on the start date. */
		if (!workStarted)
			start = day;
		double remaining;
		if ((remaining = a->getResource()->isAvailable(day)) > 0.0)
		{
			if (remaining > (a->getLoad() / 100.0))
				remaining = a->getLoad() / 100.0;
			if (remaining > (effort - done))
				remaining = effort - done;
			if (a->getResource()->book(new Booking(
				day, this, remaining)))
			{
				addBookedResource(a->getResource());
				done += remaining;
				costs += a->getResource()->getRate() * remaining;
			}
			allocFound = TRUE;
		}
		else
		{
//			fprintf(stderr,
//					"Resource %s cannot be used for task '%s' on %s.\n",
//					a->getResource()->getId().latin1(),
//					id.latin1(), time2ISO(day).latin1());
			for (Resource* r = a->first(); r != 0; r = a->next())
				if ((remaining = r->isAvailable(day)) > 0.0)
				{
					if (remaining > (a->getLoad() / 100.0))
						remaining = a->getLoad() / 100.0;
					if (remaining > (effort - done))
						remaining = effort - done;
					if (r->book(new Booking(
						day, this, remaining)))
					{
						addBookedResource(a->getResource());
						done += remaining;
						costs += r->getRate() * remaining;
					}
					allocFound = TRUE;
					break;
				}
		}
	}
	if (allocFound)
		workStarted = TRUE;

	return allocFound;
}

bool
Task::isScheduled()
{
	return ((start != 0 && end != 0) || !subTasks.isEmpty());
}

time_t
Task::earliestStart()
{
	time_t day = 0;
	for (Task* t = previous.first(); t != 0; t = previous.next())
		if (t->getEnd() > day)
			day = t->getEnd();

	/* If the task duration is enforced by length (and not effort) a task
	 * starts the next working day after the previous tasks have been
	 * finished. With effort based scheduling we can schedule multiple
	 * tasks per day. */
	if (day > 0 && length > 0)
		day = nextWorkingDay(day);

	return day;
}

bool
Task::xRef(QDict<Task>& hash)
{
	bool error = FALSE;

	for (QStringList::Iterator it = depends.begin(); it != depends.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			fatalError(QString("Unknown dependency '") + absId + "'");
			error = TRUE;
		}
		else if (previous.find(t) != -1)
		{
			fatalError(QString("No need to specify dependency '") + absId +
							   "' twice.");
			error = TRUE;
		}
		else
		{
			previous.append(t);
			t->addFollower(this);
		}
	}

	return error;
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
		if (!t->parent)
		{
			fatalError(QString("Illegal relative ID '") + relId + "'");
			return relId;
		}
		t = t->parent;
	}
	if (t)
		return t->id + "." + relId.right(relId.length() - i);
	else
		return relId.right(relId.length() - i);
}

bool
Task::isWorkingDay(time_t d) const
{
	struct tm* tms = localtime(&d);
	// Saturday and Sunday are days off.
	if (tms->tm_wday < 1 || tms->tm_wday > 5)
		return FALSE;

	return !project->isVacationDay(d);
}

time_t
Task::nextWorkingDay(time_t d) const
{
	d += 60 * 60 * 24;
	while (!isWorkingDay(d))
		d += 60 * 60 * 24;
	return d;
}

bool
Task::scheduleOK()
{
	if (!subTasks.isEmpty())
		return TRUE;

	if (start == 0)
	{
		fatalError(QString("Task '") + id + "' has no start time.");
		return false;
	}
	if (end == 0)
	{
		fatalError(QString("Task '") + id + "' has no end time.");
		return false;
	}
	// Check if all previous tasks end before start of this task.
	for (Task* t = previous.first(); t != 0; t = previous.next())
		if (t->getEnd() > start)
		{
			fatalError(QString("Task '") + id + "' cannot follow task '" +
					   t->getId() + "'.");
			return false;
		}
	// Check if all following task start after this tasks end.
	for (Task* t = followers.first(); t != 0; t = followers.next())
		if (end > t->getStart())
		{
			fatalError(QString("Task '") + id + "' cannot preceed task '" +
					   t->getId() + "'.");
			return false;
		}

	return TRUE;
}

TaskList::TaskList()
{
}

TaskList::~TaskList()
{
}

int
TaskList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Task* t1 = static_cast<Task*>(i1);
	Task* t2 = static_cast<Task*>(i2);

	switch (sorting)
	{
	case PrioUp:
		if (t1->getPriority() == t2->getPriority())
			return 0; // TODO: Use duration as next criteria
		else
			return (t1->getPriority() - t2->getPriority());
	case PrioDown:
		if (t1->getPriority() == t2->getPriority())
			return 0; // TODO: Use duration as next criteria
		else
			return (t2->getPriority() - t1->getPriority());
	default:
		fprintf(stderr, "Unknown sorting criteria!\n");
		return 0;
	}		
}
