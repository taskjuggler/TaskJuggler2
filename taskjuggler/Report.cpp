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
	resourceSortCriteria = ResourceList::ResourceTree;
	hideTask = 0;
	rollUpTask = 0;
	hideResource = 0;
	rollUpResource = 0;

	showActual = FALSE;
}

Report::~Report()
{
	delete hideTask;
	delete rollUpTask;
	delete hideResource;
	delete rollUpResource;
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

bool
Report::isResourceRolledUp(Resource* r)
{
	if (!rollUpResource)
		return FALSE;

	rollUpResource->clearSymbolTable();
	QStringList flags = *r;
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		rollUpResource->registerSymbol(*it, 1);
	return rollUpResource->eval() != 0;
}

void
Report::setHideTask(ExpressionTree* et)
{
	delete hideTask;
	hideTask = et;
}

void
Report::setRollUpTask(ExpressionTree* et)
{
	delete rollUpTask;
	rollUpTask = et; 
}

void
Report::setHideResource(ExpressionTree* et)
{
	delete hideResource;
	hideResource = et; 
}

void
Report::setRollUpResource(ExpressionTree* et)
{
	delete rollUpResource;
	rollUpResource = et;
}

void
Report::filterTaskList(TaskList& filteredList, Resource* r)
{
	/* Create a new list that contains only those tasks that were not
	 * hidden. */
	TaskList taskList = project->getTaskList();
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		Interval iv(start, end);
		if (!isTaskHidden(t) &&
			iv.overlaps(Interval(t->getPlanStart(), t->getPlanEnd())) &&
			(r == 0 || r->getPlanLoad(Interval(start, end), t) > 0.0 ||
			 (showActual && r->getActualLoad(Interval(start, end), t) > 0.0)))

			filteredList.append(t);
	}

	/* Now we have to remove all sub tasks of task in the roll-up list
     * from the filtered list */
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
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

void
Report::filterResourceList(ResourceList& filteredList, Task* t)
{
	/* Create a new list that contains only those resources that were
	 * not hidden. */
	ResourceList resourceList = project->getResourceList();
	for (Resource* r = resourceList.first(); r != 0;
		 r = resourceList.next())
	{
		if (!isResourceHidden(r) &&
			(t == 0 || r->getPlanLoad(Interval(start, end), t) > 0.0 ||
			 (showActual && r->getActualLoad(Interval(start, end), t) > 0.0)))
		{
			filteredList.append(r);
		}
	}

	/* Now we have to remove all sub resources of resource in the
     * roll-up list from the filtered list */
	for (Resource* r = resourceList.first(); r != 0;
		 r = resourceList.next())
	{
		ResourceList toHide;
		if (isResourceRolledUp(r))
			r->getSubResourceList(toHide);

		for (Resource* l = toHide.first(); l != 0; l = toHide.next())
			filteredList.remove(l);
	}
}

void
Report::sortResourceList(ResourceList& filteredList)
{
	/* In resourcetree sorting mode we need to make sure that we don't
	 * hide parents of shown resources. */
	if (resourceSortCriteria == ResourceList::ResourceTree)
	{
		filteredList.setSorting(ResourceList::Pointer);
		for (Resource* r = filteredList.first(); r != 0;
			 r = filteredList.next())
		{
			for (Resource* p = r->getParent(); p != 0; p = p->getParent())
				if (filteredList.contains(p) == 0)
					filteredList.append(p);
		}
	}

	filteredList.setSorting(resourceSortCriteria);
	filteredList.sort();
}

void
Report::scaleTime(double t, bool verboseUnit)
{
	double dwh = project->getDailyWorkingHours();
	QStringList variations;
	QValueList<double> factors;
	const char* shortUnit[] = { "d", "h", "w", "m", "y" };
	const char* unit[] = { "Day", "Hour", "Week", "Month", "Year" };
	const char* units[] = { "Days", "Hours", "Weeks", "Months", "Years"};
	double max[] = { 0, 48, 8, 24, 0 };

	factors.append(1);
	factors.append(dwh);
	factors.append(1.0 / 7);
	factors.append(1.0 / 30);
	factors.append(1.0 / 365);

	for (QValueList<double>::Iterator it = factors.begin();
		 it != factors.end(); ++it)
	{
		QString str;
		str.sprintf("%.1f", t * *it);
		// If str ends with ".0" remove ".0".
		if (str[str.length() - 1] == '0')
			str = str.left(str.length() - 2);
		int idx = factors.findIndex(*it);
		if ((*it != 1.0 && str == "0") ||
			(max[idx] != 0 && max[idx] < (t * *it)))
			variations.append("");
		else
			variations.append(str);
	}

	uint shortest = 0;
	for (QStringList::Iterator it = variations.begin(); it != variations.end();
		 ++it)
	{
		if ((*it).length() > 0 &&
			(*it).length() - ((*it).find('.') < 0 ? 0 : 1) <
			variations[shortest].length() -
			(variations[shortest].find('.') < 0 ? 0 : 1))
		{
			shortest = variations.findIndex(*it);
		}
	}
	s << variations[shortest];
	if (verboseUnit)
	{
		if (variations[shortest] == "1")
			s << " " << unit[shortest];
		else
			s << " " << units[shortest];
	}
	else
		s << shortUnit[shortest];
}
