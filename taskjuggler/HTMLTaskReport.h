/*
 * HTMLTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLTaskReport_h_
#define _HTMLTaskReport_h_

#include "ReportHtml.h"
#include "Task.h"

class Project;

class HTMLTaskReport : public ReportHtml
{
public:
	HTMLTaskReport(Project* p, const QString& f, time_t s, time_t e) :
		ReportHtml(p, f, s, e)
	{
		showActual = FALSE;
	}
	~HTMLTaskReport();

	bool generate();

	void setShowActual(bool s) { showActual = s; }

private:
	HTMLTaskReport() { }

	bool generateTableHeader();

	void generateTaskName(Task* t);

	void generateResources(Task* t);
	void generateDepends(Task* t, const QDict<int>& idxDict);
	void generateFollows(Task* t, const QDict<int>& idxDict);

	void generateDailyPlan(Task* t);
	void generateDailyActual(Task* t);

	void generateWeeklyPlan(Task* t);
	void generateWeeklyActual(Task* t);

	void generateMonthlyPlan(Task* t);
	void generateMonthlyActual(Task* t);

	bool showActual;
} ;

#endif
