/*
 * Project.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>
#include <stdio.h>

#include <qdict.h>

#include "Project.h"
#include "Utility.h"

#define COL_DEFAULT "#fffadd"
#define COL_WEEKEND "#ffec80"
#define COL_BOOKED "#ffc0a3"
#define COL_HEADER "#a5c2ff"
#define COL_MILESTONE "#ff2a2a"
#define COL_COMPLETED "#a1ff9a"
#define COL_TODAY "#a387ff"

Project::Project()
{
	taskList.setAutoDelete(TRUE);
	priority = 500;
	dailyWorkingHours = 8.0;
	scheduleGranularity = ONEHOUR;
	start = 0;
	end = 0;
	now = time(0);
	copyright = "";
	minEffort = 0.0;
	maxEffort = 1.0;
	rate = 0.0;
	xmlreport = 0L;
}

bool
Project::addTask(Task* t)
{
	taskList.append(t);
	return TRUE;
}

bool
Project::pass2()
{
	QDict<Task> idHash;
	bool error = FALSE;

	// Create hash to map task IDs to pointers.
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		idHash.insert(t->getId(), t);
	}

	// Create cross links from dependency lists.
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		if (!t->xRef(idHash))
			error = TRUE;
	}

	TaskList sortedTasks(taskList);
	sortedTasks.setAutoDelete(FALSE);
	sortedTasks.setSorting(TaskList::PrioDown);
	sortedTasks.sort();

	time_t timeDelta = scheduleGranularity;
	bool forward = TRUE;
	for (int day = start; day >= start && day < end; day += timeDelta)
	{
		bool done;
		do
		{
			done = TRUE;
			for (Task* t = sortedTasks.first(); t != 0; t = sortedTasks.next())
				if (!t->schedule(day, scheduleGranularity))
				{
					done = FALSE;
					break;	// Start with top priority tasks again.
				}
		} while (!done);

		/* If we have at least one ALAP task that has an end date but no
		 * start date then we move backwards in time. Otherwise we more
		 * forward in time. */
		timeDelta = scheduleGranularity;
		for (Task* t = sortedTasks.first(); t != 0; t = sortedTasks.next())
			if (t->needsEarlierTimeSlot(day + scheduleGranularity))
			{
				timeDelta = -scheduleGranularity;
				break;
			}
		if ((timeDelta < 0 && forward) || (timeDelta > 0 && !forward))
		{
			qDebug("Going %s at %s", timeDelta < 0 ? "backwards" : "foward",
				   time2ISO(day).latin1());
			forward = !forward;
		}
	}

	if (unscheduledTasks() > 0)
		error = TRUE;
	else
		checkSchedule();

	return error;
}

int
Project::unscheduledTasks()
{
	int cntr = 0;
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		if (!t->isScheduled())
		{
			qWarning("Task %s cannot be scheduled", t->getId().latin1());
			cntr++;
		}

	return cntr;
}

bool
Project::checkSchedule()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		if (!t->scheduleOK())
			return FALSE;

	return TRUE;
}

void
Project::generateReports()
{
	// Generate task reports
	for (HTMLTaskReport* h = htmlTaskReports.first(); h != 0;
		 h = htmlTaskReports.next())
		h->generate();
	// Generate resource reports
	for (HTMLResourceReport* r = htmlResourceReports.first(); r != 0;
		 r = htmlResourceReports.next())
		r->generate();

	if( xmlreport )
	   xmlreport->generate();
}
