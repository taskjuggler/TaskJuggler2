/*
 * ReportHtml.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
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

#include "Report.h"

class Project;
class ExpressionTree;

class ReportHtml : public Report
{
public:
	ReportHtml(Project* p, const QString& f, time_t s, time_t e);
	virtual ~ReportHtml() { }

	void generatePlanTask(Task* t, Resource* r);
	void generateActualTask(Task* t, Resource* r);

	void generatePlanResource(Resource* r, Task* t);
	void generateActualResource(Resource* r, Task* t);

	void reportHTMLHeader();
	void reportHTMLFooter();

	bool generateTableHeader();

	void generateDepends(Task* t, bool light);
	void generateFollows(Task* t, bool light);
	void htmlDayHeaderDays();
	void htmlDayHeaderMonths();
	void htmlWeekHeaderWeeks();
	void htmlWeekHeaderMonths();
	void htmlMonthHeaderMonths();
	void htmlMonthHeaderYears();

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

	void planSchedule(Resource* r, Task* t);
	void actualSchedule(Resource* r, Task* t);

protected:
	ReportHtml() { }

	QString htmlFilter(const QString& s);

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
} ;

#endif
