/*
 * HTMLAccountReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLAccountReport_h_
#define _HTMLAccountReport_h_

#include "ReportHtml.h"
#include "qmap.h"

class Account;

/**
 * @short Stores all information about an HTML account report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLAccountReport : public ReportHtml
{
public:
	HTMLAccountReport(Project* p, const QString& f, time_t s, time_t e,
					  const QString& df, int dl);
	virtual ~HTMLAccountReport() { }

	void setAccumulate(bool s) { accumulate = s; }

	bool generate();

	void generatePlanAccount(Account* a);
	void generateActualAccount(Account* a);

	bool generateTableHeader();
	void generateTotals(const QString& label, const QString& style);

	void dailyAccountPlan(Account* a, const QString& style);
	void dailyAccountActual(Account* a, const QString& style);

	void weeklyAccountPlan(Account* a, const QString& style);
	void weeklyAccountActual(Account* a, const QString& style);

	void monthlyAccountPlan(Account* a, const QString& style);
	void monthlyAccountActual(Account* a, const QString& style);

	void quarterlyAccountPlan(Account* a, const QString& style);
	void quarterlyAccountActual(Account* a, const QString& style);

	void yearlyAccountPlan(Account* a, const QString& style);
	void yearlyAccountActual(Account* a, const QString& style);

	void reportValue(double value, const QString& bgcol, bool bold);

private:
	HTMLAccountReport() { } // Don't use this.
	void accountName(Account* a);
	QMap<QString, double> planTotals;
	QMap<QString, double> actualTotals;
	QMap<QString, double> planTotalsCosts;
	QMap<QString, double> actualTotalsCosts;
	QMap<QString, double> planTotalsRevenue;
	QMap<QString, double> actualTotalsRevenue;

	bool accumulate;
} ;

#endif
