/*
 * ReportHtml.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
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

class Project;
class ExpressionTree;

class ReportHtml : public Report
{
public:
	ReportHtml(Project* p, const QString& f, time_t s, time_t e,
			   const QString& df, int dl);
	virtual ~ReportHtml() { }

	enum BarLabelText { BLT_EMPTY = 0, BLT_LOAD };

	void generatePlanTask(Task* t, Resource* r, uint no);
	void generateActualTask(Task* t, Resource* r);

	void generatePlanResource(Resource* r, Task* t, uint no);
	void generateActualResource(Resource* r, Task* t);

	void reportHTMLHeader();
	void reportHTMLFooter();

	bool generateTableHeader();

	void generateDepends(Task* t, bool light);
	void generateFollows(Task* t, bool light);
	void generateResponsibilities(Resource* r, bool light);
	void htmlDailyHeaderDays(bool highlightNow = TRUE);
	void htmlDailyHeaderMonths();
	void htmlWeeklyHeaderWeeks(bool highlightNow = TRUE);
	void htmlWeeklyHeaderMonths();
	void htmlMonthlyHeaderMonths(bool highlightNow = TRUE);
	void htmlMonthlyHeaderYears();

	void emptyPlan(bool light);
	void emptyActual(bool light);

	void textOneRow(const QString& text, bool light, const QString& align);
	void textTwoRows(const QString& text, bool light, const QString& align);

	void dailyResourcePlan(Resource* r, Task* t);
	void dailyResourceActual(Resource* r, Task* t);
	void dailyTaskPlan(Task* t, Resource* r);
	void dailyTaskActual(Task* t, Resource* r);

	void weeklyResourcePlan(Resource* r, Task* t);
	void weeklyResourceActual(Resource* r, Task* t);
	void weeklyTaskPlan(Task* t, Resource* r);
	void weeklyTaskActual(Task* t, Resource* r);

	void monthlyResourcePlan(Resource* r, Task* t);
	void monthlyResourceActual(Resource* r, Task* t);
	void monthlyTaskPlan(Task* t, Resource* r);
	void monthlyTaskActual(Task* t, Resource* r);

	void taskName(Task* t, Resource* r, bool big);
	void resourceName(Resource* t, Task* t, bool big);

 	void planResources(Task* t, bool light);
	void actualResources(Task* t, bool light);

	void reportLoad(double load, const QString& bgcol, bool bold);
	void reportPIDs(const QString& pids, const QString bgCol, bool bold);

	void generateSchedule(int sc, Resource* r, Task* t);

	void flagList(CoreAttributes* c1, CoreAttributes* c2);

	void setBarLabels(BarLabelText blt) { barLabels = blt; }

	void registerUrl(const QString& key, const QString& url = QString::null)
	{
		urls[key] = url;
	}
	bool setUrl(const QString& key, const QString& url);
	const QString* getUrl(const QString& key) const;

	void setRawHead(const QString& head)
	{
		rawHead = head;
	}

	void setRawTail(const QString& tail)
	{
		rawTail = tail;
	}

	void setRawStyleSheet(const QString& styleSheet)
	{
		rawStyleSheet = styleSheet;
	}
	
protected:
	ReportHtml() { }

	QString htmlFilter(const QString& s);
	QString generateUrl(const QString& key, const QString& txt);

	MacroTable mt;

	uint colDefault;
	uint colDefaultLight;
	uint colWeekend;
	uint colVacation;
	uint colAvailable;
	uint colBooked;
	uint colBookedLight;
	uint colHeader;
	uint colMilestone;
	uint colCompleted;
	uint colCompletedLight;
	uint colToday;

	BarLabelText barLabels;

	QString rawHead;
	QString rawTail;
	QString rawStyleSheet;

	QMap<QString, QString> urls;
} ;

#endif
