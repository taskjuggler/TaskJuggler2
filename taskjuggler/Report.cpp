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
	taskSortCriteria = CoreAttributesList::TreeMode;
	resourceSortCriteria = CoreAttributesList::TreeMode;
	accountSortCriteria = CoreAttributesList::TreeMode;

	hideTask = 0;
	rollUpTask = 0;
	hideResource = 0;
	rollUpResource = 0;
	hideAccount = 0;
	rollUpAccount = 0;

	hidePlan = FALSE;
	showActual = FALSE;
	showPIDs = FALSE;
}

Report::~Report()
{
	delete hideTask;
	delete rollUpTask;
	delete hideResource;
	delete rollUpResource;
}

bool
Report::isHidden(CoreAttributes* c, ExpressionTree* et)
{
	if (!et)
		return FALSE;

	et->clearSymbolTable();
	QStringList flags = c->getFlagList();
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		et->registerSymbol(*it, 1);
	return et->eval() != 0;
}

bool
Report::isRolledUp(CoreAttributes* c, ExpressionTree* et)
{
	if (!et)
		return FALSE;

	et->clearSymbolTable();
	QStringList flags = c->getFlagList();
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		et->registerSymbol(*it, 1);
	return et->eval() != 0;
}

bool
Report::isTaskRolledUp(Task* t)
{
	if (!rollUpTask)
		return FALSE;

	rollUpTask->clearSymbolTable();
	QStringList flags = t->getFlagList();
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
	QStringList flags = r->getFlagList();
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
Report::setHideAccount(ExpressionTree* et)
{
	delete hideAccount;
	hideAccount = et;
}

void
Report::setRollUpAccount(ExpressionTree* et)
{
	delete rollUpAccount;
	rollUpAccount = et;
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
		if (!isHidden(t, hideTask) &&
			iv.overlaps(Interval(t->getPlanStart(), t->getPlanEnd())) &&
			(r == 0 || r->getPlanLoad(Interval(start, end), t) > 0.0 ||
			 (showActual && r->getActualLoad(Interval(start, end), t) > 0.0)))
		{
			filteredList.append(t);
		}
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
	TaskList list = filteredList;
	if (taskSortCriteria == CoreAttributesList::TreeMode)
	{
		filteredList.setSorting(CoreAttributesList::Pointer);
		for (Task* t = filteredList.first(); t != 0;
			 t = filteredList.next())
		{
			for (Task* p = t->getParent(); p != 0; p = p->getParent())
				if (list.contains(p) == 0)
					list.append(p);
		}
	}
	filteredList = list;

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
		if (!isHidden(r, hideResource) &&
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
	ResourceList list = filteredList;
	if (resourceSortCriteria == CoreAttributesList::TreeMode)
	{
		filteredList.setSorting(ResourceList::Pointer);
		for (Resource* r = filteredList.first(); r != 0;
			 r = filteredList.next())
		{
			for (Resource* p = r->getParent(); p != 0; p = p->getParent())
				if (list.contains(p) == 0)
					list.append(p);
		}
	}
	filteredList = list;

	filteredList.setSorting(resourceSortCriteria);
	filteredList.sort();
}

void
Report::filterAccountList(AccountList& filteredList, Account::AccountType at)
{
	/* Create a new list that contains only those accounts that were not
	 * hidden. */
	filteredList.clear();
	AccountList accountList = project->getAccountList();
	for (Account* a = accountList.first(); a != 0; a = accountList.next())
	{
		if (!isHidden(a, hideAccount) && a->getType() == at)
			filteredList.append(a);
	}

	/* Now we have to remove all sub accounts of account in the roll-up list
     * from the filtered list */
	for (Account* a = accountList.first(); a != 0; a = accountList.next())
	{
		AccountList toHide;
		if (isRolledUp(a, rollUpAccount))
			toHide = a->getSubList();

		for (Account* a = toHide.first(); a != 0; a = toHide.next())
			filteredList.remove(a);
	}
}

void
Report::sortAccountList(AccountList& filteredList)
{
	/* In accounttree sorting mode we need to make sure that we don't hide
	 * parents of shown accounts. */
	AccountList list = filteredList;
	if (accountSortCriteria == CoreAttributesList::TreeMode)
	{
		list.setSorting(CoreAttributesList::Pointer);
		for (Account* t = filteredList.first(); t != 0;
			 t = filteredList.next())
		{
			for (Account* p = t->getParent(); p != 0; p = p->getParent())
				if (list.contains(p) == 0)
					list.append(p);
		}
	}
	filteredList = list;

	filteredList.setSorting(accountSortCriteria);
	filteredList.sort();
}

void
Report::scaleTime(double t, bool verboseUnit)
{
	double dwh = project->getDailyWorkingHours();
	QStringList variations;
	QValueList<double> factors;
	const char* shortUnit[] = { "d", "h", "w", "m", "y" };
	const char* unit[] = { "day", "hour", "week", "month", "year" };
	const char* units[] = { "days", "hours", "weeks", "months", "years"};
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
