/*
 * TaskResourceTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TaskResourceTable_h_
#define _TaskResourceTable_h_

#include <time.h>

#include <qstring.h>
#include <qvaluelist.h>

#include "ReportElement.h"
#include "TableColumn.h"
#include "CoreAttributesList.h"

class QTextStream;
class Report;
class ExpressionTree;
class TaskList;
class ResourceList;
class Task;
class Resource;
class Scenario;

/**
 * @short A report element that consists of a table listing task and
 * resources.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TaskResourceTable : public ReportElement
{
public:
	TaskResourceTable(Report* r, QTextStream& ts);
	~TaskResourceTable();

	enum LoadUnit { minutes, hours, days, weeks, months, years, shortAuto,
		longAuto };
	enum BarLabelText { BLT_EMPTY = 0, BLT_LOAD };

	void addColumn(TableColumn* c) { columns.append(c); }
	void clearColumns() { columns.clear(); }

	void setHeadline(const QString& h) { headline = h; }
	const QString& getHeadline() const { return headline; }
	
	void setStart(time_t s) { start = s; }
	time_t getStart() const { return start; }

	void setEnd(time_t e) { end = e; }
	time_t getEnd() const { return end; }
	
	void setTaskRoot(const QString& root) { taskRoot = root; }
	const QString& getTaskRoot() const { return taskRoot; }

	void setHideTask(ExpressionTree* et);
	void setHideResource(ExpressionTree* et);
	void setRollUpTask(ExpressionTree* et);
	void setRollUpResource(ExpressionTree* et);

	bool setTaskSorting(int sc, int level);
	bool setResourceSorting(int sc, int level);

	void filterTaskList(TaskList& filteredList, const Resource* r,
						ExpressionTree* hideExp, ExpressionTree* rollUpExp)
		const;
	void sortTaskList(TaskList& filteredList);

	void filterResourceList(ResourceList& filteredList, const Task* t,
							ExpressionTree* hideExp, ExpressionTree* rollUpExp)
		const;
	void sortResourceList(ResourceList& filteredList);

	void clearScenarioList() { scenarios.clear(); }
	void addScenario(Scenario* s);

	bool generateTable();

protected:
	bool isHidden(const CoreAttributes* c, ExpressionTree* et) const;
	bool isRolledUp(const CoreAttributes* c, ExpressionTree* et) const;

	Report* report;
	QTextStream& s;

	QString headline;
	QPtrList<TableColumn> columns;
	time_t start;
	time_t end;
	LoadUnit loadUnit;

	QString taskRoot;

	ExpressionTree* hideTask;
	ExpressionTree* hideResource;
	ExpressionTree* rollUpTask;
	ExpressionTree* rollUpResource;
	
	int taskSortCriteria[CoreAttributesList::maxSortingLevel];
	int resourceSortCriteria[CoreAttributesList::maxSortingLevel];
	/* The maximum of the tree that we have to report in tree-sorting mode. */
	uint maxDepthTaskList;
	uint maxDepthResourceList;

	QValueList<int> scenarios;
	BarLabelText barLabels;
} ;

#endif

