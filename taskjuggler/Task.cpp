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
Task::schedule(time_t date, time_t duration)
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
			(earliestStart() > 0 && earliestStart() <= project->getStart()))
		{
			start = project->getStart();
		}
		else if (earliestStart() > 0)
		{
			start = earliestStart();
			done = 0.0;
			costs = 0.0;
			workStarted = FALSE;
			tentativeEnd = date;
		}
		else
			return TRUE;	// Task cannot be scheduled yet.
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
		/* Do not schedule anything before the start date lies within
		 * the interval. */
		if (date + duration <= start || project->isVacation(date))
			return TRUE;
		/* The effort of the task has been specified. We have to look
		 * how much the resources can contribute over the following
		 * workings days until we have reached the specified
		 * effort. */
		if (allocations.count() == 0)
		{
			fatalError("No allocations specified for effort based task");
			return TRUE;
		}
		if (!bookResources(date, duration))
//						fprintf(stderr,
//							"No resource available for task '%s' on %s\n",
//							id.latin1(),
//							time2ISO(date).latin1())
				;
		if (done >= effort)
		{
			end = tentativeEnd;
			return FALSE;
		}
	}
	else
	{
		printf("%s %s\n", id.latin1(), time2ISO(start).latin1());
		// Task is a milestone.
		end = start;
		return FALSE;
	}
	return TRUE;
}

bool
Task::scheduleContainer()
{
	Task* t;
	time_t nstart = 0;
	time_t nend = 0;

	// Check that this is really a container task
	if ((t = subTasks.first()) && (t != 0))
	{
		/* Make sure that all sub tasks have been scheduled. If not we
		 * can't yet schedule this task. */
		if (t->getStart() == 0 || t->getEnd() == 0)
			return TRUE;
		nstart = t->getStart();
		nend = t->getEnd();
	}
	else
		return TRUE;

	for (t = subTasks.next() ; t != 0; t = subTasks.next())
	{
		/* Make sure that all sub tasks have been scheduled. If not we
		 * can't yet schedule this task. */
		if (t->getStart() == 0 || t->getEnd() == 0)
			return TRUE;

		if (t->getStart() < nstart)
			nstart = t->getStart();
		if (t->getEnd() > nend)
			nend = t->getEnd();
	}

	start = nstart;
	end = nend;
	return FALSE;
}

bool
Task::bookResources(time_t date, time_t duration)
{
	bool allocFound = FALSE;

	for (Allocation* a = allocations.first(); a != 0;
		 a = allocations.next())
	{
		if (bookResource(a->getResource(), date, duration))
			allocFound = TRUE;
		else
		{
//			fprintf(stderr,
//					"Resource %s cannot be used for task '%s' on %s.\n",
//					a->getResource()->getId().latin1(),
//					id.latin1(), time2ISO(date).latin1());
			for (Resource* r = a->first(); r != 0; r = a->next())
				if (bookResource(r, date, duration))
				{
					allocFound = TRUE;
					break;
				}
		}
	}

	return allocFound;
}

bool
Task::bookResource(Resource* r, time_t date, time_t duration)
{
	Interval interval;

	if (r->isAvailable(date, duration, interval))
	{
		double intervalLoad = project->convertToDailyLoad(
			interval.getDuration());
		r->book(new Booking(interval, this,
							account ? account->getKotrusId() : QString(""),
							project->getId()));
		addBookedResource(r);

		/* Move the start date to make sure that there is
		 * some work going on on the start date. */
		if (!workStarted)
		{
			start = date;
			workStarted = TRUE;
		}

		tentativeEnd = interval.getEnd();
		done += intervalLoad;
		//costs += r->getRate() * intervalLoad;

		return TRUE;
	}
	return FALSE;
}

bool
Task::isScheduled()
{
	return ((start != 0 && end != 0) || !subTasks.isEmpty());
}

time_t
Task::earliestStart()
{
	time_t date = 0;
	for (Task* t = previous.first(); t != 0; t = previous.next())
		if (t->getEnd() > date)
			date = t->getEnd() + 1;

	/* If the task duration is enforced by length (and not effort) a task
	 * starts the next working day after the previous tasks have been
	 * finished. With effort based scheduling we can schedule multiple
	 * tasks per date. */
	if (date > 0 && length > 0)
		date = nextWorkingDay(date);

	return date;
}

double
Task::getLoadOnDay(time_t date)
{
	double load = 0.0;
	for (Resource* r = bookedResources.first(); r != 0;
		 r = bookedResources.next())
	{
		load += r->getLoadOnDay(date, this);
	}
	return load;
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

	return !project->isVacation(d);
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
