/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>
#include <stdio.h>

#include "Project.h"
#include "Report.h"
#include "Utility.h"
#include "ExpressionTree.h"

#define KW(a) a

Report::Report(Project* p, const QString& f, time_t s, time_t e,
			   const QString& df, int dl) :
		project(p), fileName(f), start(s), end(e), defFileName(df),
		defFileLine(dl)
{
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
	{
		taskSortCriteria[i] = CoreAttributesList::SequenceUp;
		resourceSortCriteria[i] = CoreAttributesList::SequenceUp;
		accountSortCriteria[i] = CoreAttributesList::SequenceUp;
	}

	weekStartsMonday = p->getWeekStartsMonday();
	timeFormat = p->getTimeFormat();
	shortTimeFormat = p->getShortTimeFormat();

	hideTask = 0;
	rollUpTask = 0;
	hideResource = 0;
	rollUpResource = 0;
	hideAccount = 0;
	rollUpAccount = 0;

	hidePlan = FALSE;
	showActual = FALSE;
	showPIDs = FALSE;

	loadUnit = days;

	maxDepthTaskList = 1;
	maxDepthResourceList = 1;
	maxDepthAccountList = 1;
}

Report::~Report()
{
	delete hideTask;
	delete rollUpTask;
	delete hideResource;
	delete rollUpResource;
	delete hideAccount;
	delete rollUpAccount;
}

bool
Report::open()
{
	if (fileName == "--" || fileName == ".")
	{
		if (!f.open(IO_WriteOnly, stdout))
		{
			qWarning("Cannout open stdout");
			return FALSE;
		}
	}
	else
	{
		f.setName(fileName);
		if (!f.open(IO_WriteOnly))
		{
			qWarning("Cannot open report file %s!\n",
					 fileName.latin1());
			return FALSE;
		}
	}
	s.setDevice(&f);
	return TRUE;
}

bool 
Report::setTaskSorting(CoreAttributesList::SortCriteria sc, int level)
{
	if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
	{
		if ((sc == CoreAttributesList::TreeMode && level > 0) ||
			!TaskList::isSupportedSortingCriteria(sc))
			return FALSE;
		taskSortCriteria[level] = sc;
	}
	else
		return FALSE;
	return TRUE;
}

bool 
Report::setResourceSorting(CoreAttributesList::SortCriteria sc, int level)
{
	if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
	{
		if ((sc == CoreAttributesList::TreeMode && level > 0) ||
			!ResourceList::isSupportedSortingCriteria(sc))
			return FALSE;
		resourceSortCriteria[level] = sc;
	}
	else
		return FALSE;
	return TRUE;
}

bool 
Report::setAccountSorting(CoreAttributesList::SortCriteria sc, int level)
{
	if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
	{
		if ((sc == CoreAttributesList::TreeMode && level > 0) ||
			!AccountList::isSupportedSortingCriteria(sc))
			return FALSE;
		accountSortCriteria[level] = sc;
	}
	else
		return FALSE;
	return TRUE;
}

bool
Report::isHidden(CoreAttributes* c, ExpressionTree* et)
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
Report::isRolledUp(CoreAttributes* c, ExpressionTree* et)
{
	if (!et)
		return FALSE;

	et->clearSymbolTable();
	QStringList flags = c->getFlagList();
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		et->registerSymbol(*it, 1);
	return et->evalAsInt(c) != 0;
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
	return rollUpTask->evalAsInt(t) != 0;
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
	return rollUpResource->evalAsInt(r) != 0;
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

bool
Report::setLoadUnit(const QString& u)
{
	if (u == KW("minutes"))
		loadUnit = minutes;
	else if (u == KW("hours"))
		loadUnit = hours;
	else if (u == KW("days"))
		loadUnit = days;
	else if (u == KW("weeks"))
		loadUnit = weeks;
	else if (u == KW("months"))
		loadUnit = months;
	else if (u == KW("years"))
		loadUnit = years;
	else if (u == KW("shortauto"))
		loadUnit = shortAuto;
	else if (u == KW("longauto"))
		loadUnit = longAuto;
	else
		return FALSE;

	return TRUE;
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
			iv.overlaps(Interval(t->getStart(Task::Plan),
								 t->isMilestone() ? t->getStart(Task::Plan) :
								 t->getEnd(Task::Plan))) &&
			(r == 0 || r->getLoad(Task::Plan, Interval(start, end), t) > 0.0 ||
			 (showActual && r->getLoad(Task::Actual, 
									   Interval(start, end), t) > 0.0)))
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
	if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		// Set sorting criteria so sequence no since list.contains() needs it.
		filteredList.setSorting(CoreAttributesList::SequenceUp, 0);
		for (Task* t = filteredList.first(); t != 0;
			 t = filteredList.next())
		{
			// Do not add the taskRoot task or any of it's parents.
			for (Task* p = t->getParent();
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
Report::filterResourceList(ResourceList& filteredList, Task* t)
{
	/* Create a new list that contains only those resources that were
	 * not hidden. */
	ResourceList resourceList = project->getResourceList();
	for (Resource* r = resourceList.first(); r != 0;
		 r = resourceList.next())
	{
		if (!isHidden(r, hideResource) &&
			(t == 0 || r->getLoad(Task::Plan, Interval(start, end), t) > 0.0 ||
			 (showActual && r->getLoad(Task::Actual, 
									   Interval(start, end), t) > 0.0)))
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
	if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		// Set sorting criteria so sequence no since list.contains() needs it.
		filteredList.setSorting(CoreAttributesList::SequenceUp, 0);
		for (Resource* r = filteredList.first(); r != 0;
			 r = filteredList.next())
		{
			for (Resource* p = r->getParent(); p != 0; p = p->getParent())
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

void
Report::filterAccountList(AccountList& filteredList, Account::AccountType at)
{
	/* Create a new list that contains only those accounts that were not
	 * hidden. */
	filteredList.clear();
	AccountList accountList = project->getAccountList();
	for (Account* a = accountList.first(); a != 0; a = accountList.next())
	{
		if (!isHidden(a, hideAccount) && a->getAcctType() == at)
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
	if (accountSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		// Set sorting criteria so sequence no since list.contains() needs it.
		list.setSorting(CoreAttributesList::SequenceUp, 0);
		for (Account* t = filteredList.first(); t != 0;
			 t = filteredList.next())
		{
			for (Account* p = t->getParent(); p != 0; p = p->getParent())
				if (list.contains(p) == 0)
					list.append(p);
		}
	}
	filteredList = list;

	for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
		filteredList.setSorting(accountSortCriteria[i], i);
	filteredList.sort();
	
	maxDepthAccountList = filteredList.maxDepth();
}

QString
Report::scaledLoad(double t)
{
	QStringList variations;
	QValueList<double> factors;
	const char* shortUnit[] = { "d", "min", "h", "w", "m", "y" };
	const char* unit[] = { "day", "minute", "hour", "week", "month", "year" };
	const char* units[] = { "days", "minutes", "hours", "weeks", "months", 
		"years"};
	double max[] = { 0, 60, 48, 8, 24, 0 };

	factors.append(1);
	factors.append(project->getDailyWorkingHours() * 60);
	factors.append(project->getDailyWorkingHours());
	factors.append(1.0 / project->getWeeklyWorkingDays());
	factors.append(1.0 / project->getMonthlyWorkingDays());
	factors.append(1.0 / project->getYearlyWorkingDays());

	QString str;
	
	if (loadUnit == shortAuto || loadUnit == longAuto)
	{
		for (QValueList<double>::Iterator it = factors.begin();
			 it != factors.end(); ++it)
		{
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
		for (QStringList::Iterator it = variations.begin();
			 it != variations.end();
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
		str = variations[shortest];
		if (loadUnit == longAuto)
		{
			if (variations[shortest] == "1")
				str += QString(" ") + unit[shortest];
			else
				str += QString(" ") + units[shortest];
		}
		else
			str += shortUnit[shortest];
	}
	else
	{
		switch (loadUnit)
		{
			case days:
				str.sprintf("%.1f", t * factors[0]);
				break;
			case minutes:
				str.sprintf("%.1f", t * factors[1]);
				break;
			case hours:
				str.sprintf("%.1f", t * factors[2]);
				break;
			case weeks:
				str.sprintf("%.1f", t * factors[3]);
				break;
			case months:
				str.sprintf("%.1f", t * factors[4]);
				break;
			case years:
				str.sprintf("%.1f", t * factors[5]);
				break;
			case shortAuto:
			case longAuto:
				break;	// handled above switch statement already
		}
		// If str ends with ".0" remove ".0".
		if (str[str.length() - 1] == '0')
			str = str.left(str.length() - 2);
	}
	return str;
}

void
Report::warningMsg(const char* msg, ... )
{
	va_list ap;
	va_start(ap, msg);
	char buf[1024];
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);
	qWarning("%s:%d:%s", defFileName.latin1(), defFileLine, buf);
}

QString
Report::stripTaskRoot(QString taskId) const
{
	if (taskRoot == taskId.left(taskRoot.length()))
		return taskId.right(taskId.length() - taskRoot.length());
	else
		return taskId;
}

