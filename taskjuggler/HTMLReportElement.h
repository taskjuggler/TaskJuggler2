/*
 * HTMLReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_Html_h_
#define _Report_Html_h_

#include <stdio.h>
#include <time.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qtextstream.h>
#include <qmap.h>

#include "Report.h"
#include "MacroTable.h"
#include "taskjuggler.h"

class Project;
class ExpressionTree;

/**
 * @short Stores all information about an HTML report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLReportElement
{
public:
	HTMLReportElement() { }
	virtual ~HTMLReportElement() { }

	enum BarLabelText { BLT_EMPTY = 0, BLT_LOAD };

	void generateTask1stRow(const Task* t, const Resource* r, uint no);
	void generateTaskNthRow(const Task* t, const Resource* r);

	void generateResource1stRow(const Resource* r, const Task* t, uint no);
	void generateResourceNthRow(const Resource* r, const Task* t);

	bool generateTableHeader();

	void generateDepends(const Task* t, bool light);
	void generateFollows(const Task* t, bool light);
	void generateResponsibilities(const Resource* r, bool light);
	void htmlDailyHeaderDays(bool highlightNow = TRUE);
	void htmlDailyHeaderMonths();
	void htmlWeeklyHeaderWeeks(bool highlightNow = TRUE);
	void htmlWeeklyHeaderMonths();
	void htmlMonthlyHeaderMonths(bool highlightNow = TRUE);
	void htmlMonthlyHeaderYears();

	void empty1stRow(bool light);
	void emptyNthRow(bool light);

	void textOneRow(const QString& text, bool light, const QString& align);
	void textMultiRows(const QString& text, bool light, const QString& align);

	void dailyResource1stRow(const Resource* r, const Task* t);
	void dailyResourceNthRow(const Resource* r, const Task* t);
	void dailyTask1stRow(const Task* t, const Resource* r);
	void dailyTaskNthRow(const Task* t, const Resource* r);

	void weeklyResource1stRow(const Resource* r, const Task* t);
	void weeklyResourceNthRow(const Resource* r, const Task* t);
	void weeklyTask1stRow(const Task* t, const Resource* r);
	void weeklyTaskNthRow(const Task* t, const Resource* r);

	void monthlyResource1stRow(const Resource* r, const Task* t);
	void monthlyResourceNthRow(const Resource* r, const Task* t);
	void monthlyTask1stRow(const Task* t, const Resource* r);
	void monthlyTaskNthRow(const Task* t, const Resource* r);

	void taskName(const Task* t, const Resource* r, bool big);
	void resourceName(const Resource* t, const Task* t, bool big);

 	void scenarioResources(int sc, const Task* t, bool light);

	void reportLoad(double load, const QString& bgcol, bool bold);
	void reportPIDs(const QString& pids, const QString bgCol, bool bold);

	void generateSchedule(int sc, const Resource* r, const Task* t);

	void flagList(const CoreAttributes* c1, const CoreAttributes* c2);

	void generateTaskStatus(TaskStatus status, bool light);

	void setBarLabels(BarLabelText blt) { barLabels = blt; }

	void registerUrl(const QString& key, const QString& url = QString::null)
	{
		urls[key] = url;
	}
	bool setUrl(const QString& key, const QString& url);
	const QString* getUrl(const QString& key) const;

protected:
	HTMLReportElement() { }

	QString htmlFilter(const QString& s) const;
	QString generateUrl(const QString& key, const QString& txt);

	MacroTable mt;

	QMap<QString, QString> urls;
} ;

#endif
