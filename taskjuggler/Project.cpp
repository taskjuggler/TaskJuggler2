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
	/* The 'closed' flag may be used for container classes to hide all
	 * sub tasks. */
	addAllowedFlag("closed");
	// The 'hidden' flag may be used to hide the task in all reports.
	addAllowedFlag("hidden");
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
	sortedTasks.setSorting(TaskList::PrioDown);
	sortedTasks.sort();

	for (int day = start; day < end; day += scheduleGranularity)
	{
		bool done;
		do
		{
			done = TRUE;
			for (Task* t = sortedTasks.first(); t != 0; t = sortedTasks.next())
				if (!t->schedule(day, scheduleGranularity))
					done = FALSE;
		} while (!done);
	}

	if (unscheduledTasks() > 0)
	{
		qWarning("Can't schedule some tasks. Giving up!");
	}
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
			cntr++;

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
}
