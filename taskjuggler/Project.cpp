/*
 * Project.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */

#include <stdio.h>

#include "Project.h"

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

	int uc = 0, puc = -1;
	while ((uc = unscheduledTasks()) > 0 && uc != puc)
	{
		for (Task* t = taskList.first(); t != 0; t = taskList.next())
			t->schedule(start);
		puc = uc;
	}

	if (uc > 0 && uc == puc)
	{
		fprintf(stderr, "Can't schedule some tasks. Giving up!\n");
	}
	else
		checkSchedule();

	return error;
}

void
Project::printText()
{
	printf("ID  Task Name    Start      End\n");
	int i = 0;
	for (Task* t = taskList.first(); t != 0; t = taskList.next(), ++i)
	{
		printf("%2d. %-12s %10s %10s\n",
			   i, t->getName().latin1(),
			   t->getStartISO().latin1(), t->getEndISO().latin1());
	}

	printf("\n\n");
	resourceList.printText();
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

	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->scheduleContainer();

	return TRUE;
}
