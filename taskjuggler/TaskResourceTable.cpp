/*
 * TaskResourceTable.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <qtextstream.h>

#include "TaskResourceTable.h"
#include "TaskList.h"
#include "ResourceList.h"
#include "Report.h"
#include "ExpressionTree.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Scenario.h"

TaskResourceTable::TaskResourceTable(Report* r, QTextStream& ts) :
	report(r), s(ts)
{
	columns.setAutoDelete(TRUE);

	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
	{
		taskSortCriteria[i] = CoreAttributesList::SequenceUp;
		resourceSortCriteria[i] = CoreAttributesList::SequenceUp;
	}

	hideTask = 0;
	rollUpTask = 0;
	hideResource = 0;
	rollUpResource = 0;

	loadUnit = days;

	maxDepthTaskList = 1;
	maxDepthResourceList = 1;
}

TaskResourceTable::~TaskResourceTable()
{
	delete hideTask;
	delete rollUpTask;
	delete hideResource;
	delete rollUpResource;
}

void
TaskResourceTable::setHideTask(ExpressionTree* et)
{
	delete hideTask;
	hideTask = et;
}

void
TaskResourceTable::setRollUpTask(ExpressionTree* et)
{
	delete rollUpTask;
	rollUpTask = et; 
}

void
TaskResourceTable::setHideResource(ExpressionTree* et)
{
	delete hideResource;
	hideResource = et; 
}

void
TaskResourceTable::setRollUpResource(ExpressionTree* et)
{
	delete rollUpResource;
	rollUpResource = et;
}

bool 
TaskResourceTable::setTaskSorting(int sc, int level)
{
	if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
	{
		if ((sc == CoreAttributesList::TreeMode && level > 0) ||
			!TaskList::isSupportedSortingCriteria(sc & 0xFFFF))
			return FALSE;
		taskSortCriteria[level] = sc;
	}
	else
		return FALSE;
	return TRUE;
}

bool 
TaskResourceTable::setResourceSorting(int sc, int level)
{
	if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
	{
		if ((sc == CoreAttributesList::TreeMode && level > 0) ||
			!ResourceList::isSupportedSortingCriteria(sc & 0xFFFF))
			return FALSE;
		resourceSortCriteria[level] = sc;
	}
	else
		return FALSE;
	return TRUE;
}

void
TaskResourceTable::filterTaskList(TaskList& filteredList, const Resource* r,
								  ExpressionTree* hideExp, 
								  ExpressionTree* rollUpExp) const
{
	/* Create a new list that contains only those tasks that were not
	 * hidden. */
	filteredList.clear();
	for (TaskListIterator tli(report->getProject()->getTaskListIterator());
		 *tli != 0; ++tli)
	{
		Interval iv(start, end);
		bool taskOverlapsInAnyScenario = FALSE;
		bool resourceIsLoadedInAnyScenario = (r == 0);
		QValueList<int>::const_iterator it;
		for (it = scenarios.begin(); it != scenarios.end(); ++it)
		{
			if (iv.overlaps(Interval((*tli)->getStart(*it),
									 (*tli)->isMilestone() ?
								   	 (*tli)->getStart(*it) :
									 (*tli)->getEnd(*it))))
			{
				taskOverlapsInAnyScenario = TRUE;
				break;
			}
			if (!resourceIsLoadedInAnyScenario && 
				r->getLoad(*it, iv, AllAccounts, *tli) > 0.0)
			{
				resourceIsLoadedInAnyScenario = TRUE;
			}
		}
		
		if (!isHidden(*tli, hideExp) &&
			taskOverlapsInAnyScenario && resourceIsLoadedInAnyScenario)
			filteredList.append(tli);
	}

	/* Now we have to remove all sub tasks of task in the roll-up list
     * from the filtered list */
	for (TaskListIterator tli(report->getProject()->getTaskListIterator()); 
		 *tli != 0; ++tli)
		if (isRolledUp(*tli, rollUpExp))
			for (TaskListIterator thi((*tli)->getSubListIterator()); 
				 *thi != 0; ++thi)
				filteredList.remove(*thi);
}

void
TaskResourceTable::sortTaskList(TaskList& filteredList)
{
	/* In tasktree sorting mode we need to make sure that we don't hide
	 * parents of shown tasks. */
	TaskList list = filteredList;
	if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		// Set sorting criteria so sequence no since list.contains() needs it.
		filteredList.setSorting(CoreAttributesList::SequenceUp, 0);
		for (TaskListIterator tli(filteredList); *tli != 0; ++tli)
		{
			// Do not add the taskRoot task or any of it's parents.
			for (Task* p = (*tli)->getParent();
				 p != 0 && (p->getId() + "." != taskRoot);
				 p = p->getParent())
				if (list.contains(p) == 0)
					list.append(p);
		}
	}
	filteredList = list;

	for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
		filteredList.setSorting(taskSortCriteria[i], i);
	filteredList.sort();

	maxDepthTaskList = filteredList.maxDepth();
}

void
TaskResourceTable::filterResourceList(ResourceList& filteredList, const Task* t,
									  ExpressionTree* hideExp, 
									  ExpressionTree* rollUpExp) const
{
	/* Create a new list that contains only those resources that were
	 * not hidden. */
	filteredList.clear();
	for (ResourceListIterator rli(report->getProject()->
								  getResourceListIterator());
		 *rli != 0; ++rli)
	{
		Interval iv(start, end);
		bool resourceIsLoadedInAnyScenario = (t == 0);
		if (!resourceIsLoadedInAnyScenario)
		{
			QValueList<int>::const_iterator it;
			for (it = scenarios.begin(); it != scenarios.end(); ++it)
			{
				if ((*rli)->getLoad(*it, iv, AllAccounts, t) > 0.0)
				{
					resourceIsLoadedInAnyScenario = TRUE;
				}
			}
		}
		
		if (!isHidden(*rli, hideExp) && resourceIsLoadedInAnyScenario)
			filteredList.append(*rli);
	}

	/* Now we have to remove all sub resources of resource in the
     * roll-up list from the filtered list */
	for (ResourceListIterator rli(report->getProject()->
								  getResourceListIterator());
		 *rli != 0; ++rli)
		if (isRolledUp(*rli, rollUpExp))
			for (ResourceListIterator thi((*rli)->getSubListIterator());
				 *thi != 0; ++thi)
				filteredList.remove(*thi);
}

void
TaskResourceTable::sortResourceList(ResourceList& filteredList)
{
	/* In resourcetree sorting mode we need to make sure that we don't
	 * hide parents of shown resources. */
	ResourceList list = filteredList;
	if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		// Set sorting criteria so sequence no since list.contains() needs it.
		filteredList.setSorting(CoreAttributesList::SequenceUp, 0);
		for (ResourceListIterator rli(filteredList); *rli != 0; ++rli)
		{
			for (Resource* p = (*rli)->getParent(); p != 0; p = p->getParent())
				if (list.contains(p) == 0)
					list.append(p);
		}
	}
	filteredList = list;

	for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
		filteredList.setSorting(resourceSortCriteria[i], i);
	filteredList.sort();
	
	maxDepthResourceList = filteredList.maxDepth();
}

bool
TaskResourceTable::isHidden(const CoreAttributes* c, ExpressionTree* et) const
{
	if (!taskRoot.isEmpty() &&
		strcmp(c->getType(), "Task") == 0 &&
		taskRoot != c->getId().left(taskRoot.length()))
	{
		return TRUE;
	}
	
	if (!et)
		return FALSE;

	et->clearSymbolTable();
	QStringList flags = c->getFlagList();
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		et->registerSymbol(*it, 1);
	return et->evalAsInt(c) != 0;
}

bool
TaskResourceTable::isRolledUp(const CoreAttributes* c, ExpressionTree* et) const
{
	if (!et)
		return FALSE;

	et->clearSymbolTable();
	QStringList flags = c->getFlagList();
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		et->registerSymbol(*it, 1);
	return et->evalAsInt(c) != 0;
}

void
TaskResourceTable::addScenario(Scenario* sc)
{
	scenarios.append(sc->getIndex());
}

bool
TaskResourceTable::generateTable()
{
	if (!headline.isEmpty())
		s << "<h3>" << ((ReportHtml*) report)->htmlFilter(headline) 
			<< "</h3>" << endl;
	TaskList filteredTaskList;
	filterTaskList(filteredTaskList, 0, hideTask, rollUpTask);
	sortTaskList(filteredTaskList);
	
	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
	sortResourceList(filteredResourceList);
	
	if (filteredTaskList.count() == 0)
		s << "<p>None.</p>" << endl;
	else
	{
		// TODO: Do some real stuff here.
	}
	s << "<br>" << endl;
	return FALSE;
}
