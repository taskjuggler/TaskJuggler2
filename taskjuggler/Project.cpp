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

Project::Project()
{
	taskList.setAutoDelete(TRUE);
	resourceList.setAutoDelete(TRUE);
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

	taskList.createIndex();
	resourceList.createIndex();
	accountList.createIndex();

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

	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		if (!t->preScheduleOk())
			error = TRUE;

	if (error)
		return FALSE;

	preparePlan();
	if (!schedule())
		return FALSE;
	finishPlan();

	prepareActual();
	if (!schedule())
		return FALSE;
	finishActual();

	return TRUE;
}

void
Project::preparePlan()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->preparePlan();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->preparePlan();
}

void
Project::finishPlan()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->finishPlan();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->finishPlan();
}

void
Project::prepareActual()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->prepareActual();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->prepareActual();
}

void
Project::finishActual()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->finishActual();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->finishActual();
}

bool
Project::schedule()
{
	bool error = FALSE;

	TaskList sortedTasks(taskList);
	sortedTasks.setSorting(CoreAttributesList::PrioDown);
	sortedTasks.sort();

	time_t timeDelta = scheduleGranularity;
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
	}

//	if (unscheduledTasks() > 0)
//		error = TRUE;
//	else
		checkSchedule();

	return !error;
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
		if (!t->scheduleOk())
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

	// Generate account reports
	for (HTMLAccountReport* r = htmlAccountReports.first(); r != 0;
		 r = htmlAccountReports.next())
		r->generate();

	if( xmlreport )
	   xmlreport->generate();
}
