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

	void reportHTMLHeader();
	void reportHTMLFooter();
	void htmlDayHeaderDays();
	void htmlDayHeaderMonths();
	void htmlMonthHeaderMonths();
	void htmlMonthHeaderYears();

protected:
	ReportHtml() { }

	QString htmlFilter(const QString& s);
	
	uint colDefault;
	uint colWeekend;
	uint colVacation;
	uint colAvailable;
	uint colBooked;
	uint colHeader;
	uint colMilestone;
	uint colCompleted;
	uint colToday;
} ;

#endif
