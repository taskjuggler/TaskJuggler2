/*
 * Report.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_h_
#define _Report_h_

#include <stdio.h>
#include <time.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qtextstream.h>

class Project;
class ExpressionTree;

class Report
{
public:
	Report(Project* p, const QString& f, time_t s, time_t e);
	virtual ~Report();

	void addColumn(const QString& c) { columns.append(c); }
	const QString& columnsAt(uint idx) { return columns[idx]; }
	void clearColumns() { columns.clear(); }

	void setStart(time_t s) { start = s; }
	time_t getStart() const { return start; }
	
	void setEnd(time_t e) { end = e; }
	time_t getEnd() const { return end; }

	void setHideTask(ExpressionTree* et) { hideTask = et; }
	bool isTaskHidden(Task* t);

	void setHideResource(ExpressionTree* et) { hideTask = et; }
	bool isResourceHidden(Resource* t);

protected:
	Report() { }

	Project* project;
	QString fileName;
	QStringList columns;
	time_t start;
	time_t end;

	QTextStream s;
	ExpressionTree* hideTask;
	ExpressionTree* hideResource;
} ;

#endif
