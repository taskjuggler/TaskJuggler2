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

#include <qfile.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "HTMLAccountReport.h"
#include "Project.h"
#include "Account.h"
#include "Interval.h"
#include "Utility.h"

#define KW(a) a

HTMLAccountReport::HTMLAccountReport(Project* p, const QString& f, time_t s,
							   time_t e, const QString& df, int dl) :
	ReportHtml(p, f, s, e, df, dl)
{
	columns.append("no");
	columns.append("name");
	columns.append("total");

	accumulate = FALSE;
}

bool
HTMLAccountReport::generate()
{
	if (!open())
		return FALSE;
	reportHTMLHeader();

	if (!generateTableHeader())
		return FALSE;

	AccountList filteredList;
	filterAccountList(filteredList, Cost, hideAccount, rollUpAccount);
	sortAccountList(filteredList);

	for (AccountListIterator ali(filteredList); *ali != 0; ++ali)
	{
		if (!hidePlan)
			generatePlanAccount(*ali);
		if (showActual)
			generateActualAccount(*ali);
	}
	generateTotals(i18n("Subtotal Cost"), "headersmall");
	planTotalsCosts = planTotals;
	actualTotalsCosts = actualTotals;
	planTotals.clear();
	actualTotals.clear();

	filterAccountList(filteredList, Revenue, hideAccount, rollUpAccount);
	sortAccountList(filteredList);

	for (AccountListIterator ali(filteredList); *ali != 0; ++ali)
	{
		if (!hidePlan)
			generatePlanAccount(*ali);
		if (showActual)
			generateActualAccount(*ali);
	}
	generateTotals(i18n("Subtotal Revenue"), "headersmall");
	planTotalsRevenue = planTotals;
	actualTotalsRevenue = actualTotals;

	QMap<QString, double>::Iterator tc;
	QMap<QString, double>::Iterator rc;
	QMap<QString, double>::Iterator it;
	for (tc = planTotalsCosts.begin(),
			 rc = planTotalsRevenue.begin(),
			 it = planTotals.begin();
		 tc != planTotalsCosts.end(); ++tc, ++rc, ++it)
	{
		*it = *rc - *tc;
	}
	for (tc = actualTotalsCosts.begin(),
			 rc = actualTotalsRevenue.begin(),
			 it = actualTotals.begin();
		  tc != actualTotalsCosts.end(); ++tc, ++rc, ++it)
	{
		*it = *rc - *tc;
	}
	generateTotals(i18n("Total"), "default");
		
	s << "</table>" << endl;
	reportHTMLFooter();

	f.close();
	return TRUE;
}

void
HTMLAccountReport::generatePlanAccount(Account* a)
{
	s << "<tr valign=\"middle\">" << endl; 
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("no"))
			textTwoRows(QString().sprintf("%d.", a->getIndex()), FALSE, "");
		else if (*it == KW("id"))
			textTwoRows(htmlFilter(a->getId()), FALSE, "left");
		else if (*it == KW("name"))
			accountName(a);
		else if (*it == KW("total"))
		{
			double value = a->getVolume(Task::Plan, Interval(start, end));
			planTotals["total"] += value;
			textOneRow(QString().sprintf("%.*f", project->getCurrencyDigits(),
										 value), FALSE, "right");
		}
		else if (*it == KW("daily"))
			dailyAccountPlan(a, "default");
		else if (*it == KW("weekly"))
			weeklyAccountPlan(a, "default");
		else if (*it == KW("monthly"))
			monthlyAccountPlan(a, "default");
		else if (*it == KW("quarterly"))
			quarterlyAccountPlan(a, "default");
		else if (*it == KW("yearly"))
			yearlyAccountPlan(a, "default");
		else
			qFatal("generatePlanAccount: Unknown Column %s",
				   (*it).latin1());
	}
	s << "</tr>" << endl;
}

void
HTMLAccountReport::generateActualAccount(Account* a)
{
	s << "<tr bgcolor=\"" << colAvailable << "\">" << endl;
	for (QStringList::Iterator it = columns.begin();
		 it != columns.end();
		 ++it )
	{
		if (*it == KW("no"))
		{
			if (hidePlan)
				textOneRow(QString().sprintf("%d.", a->getIndex()), FALSE, "");
		}
		else if (*it == KW("id"))
		{
			if (hidePlan)
				textOneRow(htmlFilter(a->getId()), FALSE, "left");
		}
		else if (*it == KW("name"))
		{
			if (hidePlan)
				accountName(a);
		}
		else if (*it == KW("total"))
		{
			double value = a->getVolume(Task::Actual, Interval(start, end));
			actualTotals["total"] += value;
			textOneRow(QString().sprintf("%.*f", project->getCurrencyDigits(),
										 value), FALSE, "right");
		}
		else if (*it == KW("daily"))
			dailyAccountActual(a, "default");
		else if (*it == KW("weekly"))
			weeklyAccountActual(a, "default");
		else if (*it == KW("monthly"))
			monthlyAccountActual(a, "default");
		else if (*it == KW("quarterly"))
			quarterlyAccountActual(a, "default");
		else if (*it == KW("yearly"))
			yearlyAccountActual(a, "default");
	}
	s << "</tr>" << endl;
}

bool
HTMLAccountReport::generateTableHeader()
{
	// Header line 1
	s << "<table align=\"center\" cellpadding=\"1\">\n" << endl;
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("seqno"))
			s << "<td class=\"headerbig\" rowspan=\"2\">" 
				<< i18n("Seq. No.") << "</td>";
		else if (*it == KW("no"))
			s << "<td class=\"headerbig\" rowspan=\"2\">"
				<< i18n("No.") << "</td>";
		else if (*it == KW("index"))
			s << "<td class=\"headerbig\" rowspan=\"2\">"
				<< i18n("Index No.") << "</td>";
		else if (*it == KW("id"))
			s << "<td class=\"headerbig\" rowspan=\"2\">"
				<< i18n("ID") << "</td>";
		else if (*it == KW("name"))
			s << "<td class=\"headerbig\" rowspan=\"2\">"
				<< i18n("Name") << "</td>";
		else if (*it == KW("total"))
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">"
				<< i18n("Total");
			if (!project->getCurrency().isEmpty())
				s << " " << htmlFilter(project->getCurrency());
			s << "</td>";
		}
		else if (*it == KW("daily"))
			htmlDailyHeaderMonths();
		else if (*it == KW("weekly"))
			htmlWeeklyHeaderMonths();
		else if (*it == KW("monthly"))
			htmlMonthlyHeaderYears();
		else if (*it == KW("quarterly"))
			htmlQuarterlyHeaderYears();
		else if (*it == KW("yearly"))
			htmlYearHeader();
		else
		{
			TJMH.errorMessage
				(i18n("Unknown Column '%1' for HTML Account Report")
				 .arg(*it));
			return FALSE;
		}
	}
	s << "</tr>" << endl;

	// Header line 2
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("daily"))
			htmlDailyHeaderDays(FALSE);
		else if (*it == KW("weekly"))
			htmlWeeklyHeaderWeeks(FALSE);
		else if (*it == KW("monthly"))
			htmlMonthlyHeaderMonths(FALSE);
		else if (*it == KW("quarterly"))
			htmlQuarterlyHeaderQuarters(FALSE);
	}
	s << "</tr>\n" << endl;

	return TRUE;
}

void
HTMLAccountReport::generateTotals(const QString& label, const QString& style)
{
	// Total plan values
	if (!hidePlan)
	{
		s << "<tr>" << endl;
		for (QStringList::Iterator it = columns.begin(); it != columns.end();
			 ++it )
		{
			if (*it == KW("no"))
				s << "<td class=\"" << style << "\" rowspan=\""
				  << (!hidePlan && showActual ? "2" : "1")
				  << "\">&nbsp;</td>";
			else if (*it == KW("id"))
				s << "<td class=\"" << style << "\" rowspan=\""
				  << (!hidePlan && showActual ? "2" : "1")
				  << "\">&nbsp;</td>";
			else if (*it == KW("name"))
				s << "<td class=\"" << style << "\" rowspan=\""
				  << (!hidePlan && showActual ? "2" : "1")
				  << "\" nowrap><b>" << label << "</b></td>";
			else if (*it == KW("total"))
				s << "<td class=\"" << style
				  << "\" style=\"text-align:right\">"
				  << "<b>"
				  << QString().sprintf("%.*f", project->getCurrencyDigits(),
									   planTotals["total"])
				  << "</b></td>";
			else if (*it == KW("daily"))
				dailyAccountPlan(0, style);
			else if (*it == KW("weekly"))
				weeklyAccountPlan(0, style);
			else if (*it == KW("monthly"))
				monthlyAccountPlan(0, style);
			else if (*it == KW("quarterly"))
				quarterlyAccountPlan(0, style);
			else if (*it == KW("yearly"))
				yearlyAccountPlan(0, style);
		}
		s << "</tr>" << endl;
	}

	if (showActual)
	{
		// Total actual values
		s << "<tr>" << endl;
		for (QStringList::Iterator it = columns.begin(); it != columns.end();
			 ++it )
		{
			if (*it == KW("no"))
			{
				if (hidePlan)
					s << "<td class=\"" << style << "\">&nbsp;</td>";
			}
			else if (*it == KW("id"))
			{
				if (hidePlan)
					s << "<td class=\"" << style << "\">&nbsp;</td>";
			}
			else if (*it == KW("name"))
			{
				if (hidePlan)
					s << "<td class=\"" << style << "\" nowrap><b>"
					  << label << "</b></td>";
			}
			else if (*it == KW("total"))
				s << "<td class=\"" << style
				  << "\" style=\"text-align:right\">"
				  << "<b>"
				  << QString().sprintf("%.*f", project->getCurrencyDigits(),
									   actualTotals["total"])
				  << "</b></td>";
			else if (*it == KW("daily"))
				dailyAccountActual(0, style);
			else if (*it == KW("weekly"))
				weeklyAccountActual(0, style);
			else if (*it == KW("monthly"))
				monthlyAccountActual(0, style);
			else if (*it == KW("yearly"))
				yearlyAccountActual(0, style);
		}
		s << "</tr>\n" << endl;
	}
}

void 
HTMLAccountReport::dailyAccountPlan(Account* a, const QString& style)
{
	if (hidePlan)
		return;

	if (showActual)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Plan");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Plan, Interval(day).firstDay());
			planTotals[time2ISO(day)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(planTotals[time2ISO(day)], style, TRUE);
	}
}

void
HTMLAccountReport::dailyAccountActual(Account* a, const QString& style)
{
	if (!hidePlan)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Actual");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Actual, 
										 Interval(day).firstDay());
			actualTotals[time2ISO(day)] += volume;
			reportValue(volume, "style", FALSE);
		}
		else
			reportValue(actualTotals[time2ISO(day)], style, TRUE);
	}
}

void 
HTMLAccountReport::weeklyAccountPlan(Account* a, const QString& style)
{
	if (hidePlan)
		return;

	if (showActual)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Plan");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Plan,
				accumulate ? 
				Interval(start, sameTimeNextWeek(week) - 1) :
				Interval(week).firstWeek(weekStartsMonday));
			planTotals[time2ISO(week)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(planTotals[time2ISO(week)], style, TRUE);
	}
}

void
HTMLAccountReport::weeklyAccountActual(Account* a, const QString& style)
{
	if (!hidePlan)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Actual");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Actual,
				accumulate ?
				Interval(start, sameTimeNextWeek(week) - 1) :
				Interval(week).firstWeek(weekStartsMonday));
			actualTotals[time2ISO(week)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(actualTotals[time2ISO(week)], style, TRUE);
	}
}

void
HTMLAccountReport::monthlyAccountPlan(Account* a, const QString& style)
{
	if (hidePlan)
		return;

	if (showActual)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Plan");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Plan, 
				accumulate ?
				Interval(start, sameTimeNextMonth(month) - 1) :
				Interval(month).firstMonth());
			planTotals[time2ISO(month)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(planTotals[time2ISO(month)], style, TRUE);
	}
}

void
HTMLAccountReport::monthlyAccountActual(Account* a, const QString& style)
{
	if (!hidePlan)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Actual");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Actual,
				accumulate ?
				Interval(start, sameTimeNextMonth(month) - 1) :
				Interval(month).firstMonth());
			actualTotals[time2ISO(month)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(actualTotals[time2ISO(month)], style, TRUE);
	}
}

void
HTMLAccountReport::quarterlyAccountPlan(Account* a, const QString& style)
{
	if (hidePlan)
		return;

	if (showActual)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Plan");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t quarter = beginOfQuarter(start); quarter < end;
		 quarter = sameTimeNextQuarter(quarter))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Plan, 
				accumulate ?
				Interval(start, sameTimeNextQuarter(quarter) - 1) :
				Interval(quarter).firstQuarter());
			planTotals[time2ISO(quarter)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(planTotals[time2ISO(quarter)], style, TRUE);
	}
}

void
HTMLAccountReport::quarterlyAccountActual(Account* a, const QString& style)
{
	if (!hidePlan)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Actual");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t quarter = beginOfQuarter(start); quarter < end;
		 quarter = sameTimeNextQuarter(quarter))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Actual,
				accumulate ?
				Interval(start, sameTimeNextQuarter(quarter) - 1) :
				Interval(quarter).firstQuarter());
			actualTotals[time2ISO(quarter)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(actualTotals[time2ISO(quarter)], style, TRUE);
	}
}

void
HTMLAccountReport::yearlyAccountPlan(Account* a, const QString& style)
{
	if (hidePlan)
		return;

	if (showActual)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Plan");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t year = beginOfYear(start); year < end;
		 year = sameTimeNextYear(year))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Plan, 
				accumulate ?
				Interval(start, sameTimeNextYear(year) - 1) :
				Interval(year).firstYear());
			planTotals[time2ISO(year)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(planTotals[time2ISO(year)], style, TRUE);
	}
}

void
HTMLAccountReport::yearlyAccountActual(Account* a, const QString& style)
{
	if (!hidePlan)
	{
		s << "<td class=\"headersmall\" style=\"text-align:right\">";
		if (!a)
			s << "<b>";
		s << i18n("Actual");
		if (!a)
			s << "</b>";
		s << "</td>" << endl;
	}
	for (time_t year = beginOfYear(start); year < end;
		 year = sameTimeNextYear(year))
	{
		if (a)
		{
			double volume = a->getVolume(Task::Actual,
				accumulate ?
				Interval(start, sameTimeNextYear(year) - 1) :
				Interval(year).firstYear());
			actualTotals[time2ISO(year)] += volume;
			reportValue(volume, style, FALSE);
		}
		else
			reportValue(actualTotals[time2ISO(year)], style, TRUE);
	}
}

void
HTMLAccountReport::accountName(Account* a)
{
	QString spaces;
	int fontSize = 0;

	if (accountSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		for (uint i = 0; i < a->treeLevel(); i++)
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
		fontSize = (int) (140 * (1.0 - a->treeLevel() / maxDepthResourceList));
		s << "<td class=\"default\" style=\"text-align:left\" rowspan=\""
		  << (!hidePlan && showActual ? "2" : "1") << "\" nowrap>"
		  << spaces
		  << "<font size=\""
		  << (fontSize < 0 ? '-' : '+') 
		  << (fontSize < 0 ? -fontSize : fontSize) << "\">"
		  << htmlFilter(a->getName())
		  << "</font></td>" << endl;
	}
	else
		s << "<td class=\"default\" rowspan=\""
		  << (!hidePlan && showActual ? "2" : "1")
		  << "\" style=\"text-align:left\" nowrap>"
		  << spaces << htmlFilter(a->getName())
		  << "</td>" << endl;
}

void
HTMLAccountReport::reportValue(double value, const QString& bgCol, bool bold)
{
	s << "<td class=\""
	  << bgCol << "\" style=\"text-align:right\">";
	if (bold)
		s << "<b>";
	s << QString().sprintf("%.*f", project->getCurrencyDigits(), value);
//		scaleTime(value, FALSE);
	if (bold)
		s << "</b>";
	s << "</td>" << endl;
}
