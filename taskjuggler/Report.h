/*
 * Report.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_h_
#define _Report_h_

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qfile.h>
#include <qtextstream.h>

#include <Account.h>

class Project;
class Task;
class Resource;
class Account;
class TaskList;
class ResourceList;
class AccountList;
class ExpressionTree;

#include "CoreAttributes.h"

class Report
{
public:
	Report(Project* p, const QString& f, time_t s, time_t e,
		   const QString& df, int dl);
	virtual ~Report();

	void setShowActual(bool s) { showActual = s; }
	bool getShowActual() const { return showActual; }

	void setHidePlan(bool s) { hidePlan = s; }
	void setShowPIDs(bool s) { showPIDs = s; }

	void addColumn(const QString& c) { columns.append(c); }
	const QString& columnsAt(uint idx) { return columns[idx]; }
	void clearColumns() { columns.clear(); }

	void setStart(time_t s) { start = s; }
	time_t getStart() const { return start; }
	
	void setEnd(time_t e) { end = e; }
	time_t getEnd() const { return end; }

	void setHeadline(const QString& hl) { headline = hl; }
	void setCaption(const QString& c) { caption = c; }

	bool isHidden(CoreAttributes* c, ExpressionTree* et);
	bool isRolledUp(CoreAttributes* c, ExpressionTree* et);

	void setHideTask(ExpressionTree* et);

	void setRollUpTask(ExpressionTree* et);
	bool isTaskRolledUp(Task* t);

	void setHideResource(ExpressionTree* et);

	void setHideAccount(ExpressionTree* et);

	void setRollUpResource(ExpressionTree* et);
	bool isResourceRolledUp(Resource* t);

	void setRollUpAccount(ExpressionTree* et);

	void setTaskSorting(CoreAttributesList::SortCriteria sc)
	{
		taskSortCriteria = sc;
	}
	void setResourceSorting(CoreAttributesList::SortCriteria sc)
	{
		resourceSortCriteria = sc;
	}
	void setAccountSorting(CoreAttributesList::SortCriteria sc)
	{
		accountSortCriteria = sc;
	}

	enum LoadUnit { minutes, hours, days, weeks, months, years, shortAuto,
		longAuto };

	bool setLoadUnit(const QString& u);

protected:
	Report() { }

	bool open();

	void filterTaskList(TaskList& filteredList, Resource* r);
	void sortTaskList(TaskList& filteredList);

	void filterResourceList(ResourceList& filteredList, Task* t = 0);
	void sortResourceList(ResourceList& filteredList);

	void filterAccountList(AccountList& filteredList, Account::AccountType at);
	void sortAccountList(AccountList& filteredList);

	QString scaledLoad(double t);

	void warningMsg(const char* msg, ... );

	Project* project;
	QString fileName;
	QStringList columns;
	time_t start;
	time_t end;
	/* We store the location of the report definition in case we need it
	 * for error reporting. */
	QString defFileName;
	int defFileLine;
	

	QString headline;
	QString caption;

	CoreAttributesList::SortCriteria taskSortCriteria;
	CoreAttributesList::SortCriteria resourceSortCriteria;
	CoreAttributesList::SortCriteria accountSortCriteria;

	QFile f;
	QTextStream s;
	ExpressionTree* hideTask;
	ExpressionTree* hideResource;
	ExpressionTree* hideAccount;
	ExpressionTree* rollUpTask;
	ExpressionTree* rollUpResource;
	ExpressionTree* rollUpAccount;

	LoadUnit loadUnit;

	bool hidePlan;
	bool showActual;
	bool showPIDs;
} ;

#endif
