/*
 * Report.cpp - TaskJuggler
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

#include "Project.h"
#include "Report.h"
#include "Utility.h"
#include "ExpressionTree.h"

Report::Report(Project* p, const QString& f, time_t s, time_t e) :
		project(p), fileName(f), start(s), end(e)
{
	taskSortCriteria = TaskList::TaskTree;
	hideTask = 0;
	rollUpTask = 0;
	hideResource = 0;
}

Report::~Report()
{
	delete hideTask;
	delete hideResource;
	delete rollUpTask;
}

bool
Report::isTaskHidden(Task* t)
{
	if (!hideTask)
		return FALSE;

	hideTask->clearSymbolTable();
	QStringList flags = *t;
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		hideTask->registerSymbol(*it, 1);
	return hideTask->eval() != 0;
}

bool
Report::isResourceHidden(Resource* r)
{
	if (!hideResource)
		return FALSE;

	hideResource->clearSymbolTable();
	QStringList flags = *r;
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		hideResource->registerSymbol(*it, 1);
	return hideResource->eval() != 0;
}

bool
Report::isTaskRolledUp(Task* t)
{
	if (!rollUpTask)
		return FALSE;

	rollUpTask->clearSymbolTable();
	QStringList flags = *t;
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		rollUpTask->registerSymbol(*it, 1);
	return rollUpTask->eval() != 0;
}

void
Report::filterTaskList(TaskList& filteredList)
{
	/* Created a new list that contains only those tasks that were not
	 * hidden. */
	for (Task* t = project->taskListFirst(); t != 0;
		 t = project->taskListNext())
	{
		Interval iv(start, end);
		if (!isTaskHidden(t) &&
			iv.overlaps(Interval(t->getStart(), t->getEnd())))
			filteredList.append(t);
	}

	/* Now we have to remove all sub tasks of task in the roll-up list
     * from the filtered list */
	for (Task* t = project->taskListFirst(); t != 0;
		 t = project->taskListNext())
	{
		TaskList toHide;
		if (isTaskRolledUp(t))
			t->getSubTaskList(toHide);

		for (Task* l = toHide.first(); l != 0; l = toHide.next())
			filteredList.remove(l);
	}
}

void
Report::sortTaskList(TaskList& filteredList)
{
	/* In tasktree sorting mode we need to make sure that we don't hide
	 * parents of shown tasks. */
	if (taskSortCriteria == TaskList::TaskTree)
	{
		filteredList.setSorting(TaskList::Pointer);
		for (Task* t = filteredList.first(); t != 0;
			 t = filteredList.next())
		{
			for (Task* p = t->getParent(); p != 0; p = p->getParent())
				if (filteredList.contains(p) == 0)
					filteredList.append(p);
		}
	}

	filteredList.setSorting(taskSortCriteria);
	filteredList.sort();
}

