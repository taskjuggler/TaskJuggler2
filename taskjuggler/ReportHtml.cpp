/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include "Project.h"
#include "Report.h"
#include "Utility.h"

ReportHtml::ReportHtml(Project* p, const QString& f, time_t s, time_t e) :
   Report(p, f, s, e)
{
	colDefault = 0xf3ebae;
	colDefaultLight = 0xfffadd;
	colWeekend = 0xffec80;
	colVacation = 0xfffc60;
	colAvailable = 0xa4ff8d;
	colBooked = 0xff9191;
	colBookedLight = 0xffbfbf;
	colHeader = 0xa5c2ff;
	colMilestone = 0xff2a2a;
	colCompleted = 0x87ff75;
	colCompletedLight = 0xa1ff9a;
	colToday = 0xa387ff;
}

void
ReportHtml::generatePlanTask(Task* t, Resource* r)
{
	s << "<tr valign=\"center\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "no")
			textTwoRows((r == 0 ? QString().sprintf("%d.", t->getIndex()) :
						 QString("")), r != 0, "");
		else if (*it == "id")
			textTwoRows(htmlFilter(t->getId()), r != 0, "left");
		else if (*it == "name")
			taskName(t, r, r == 0);
		else if (*it == "start")
			s << "<td class=\""
			  << (t->isPlanStartOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" nowrap>"
			  << time2ISO(t->getPlanStart())
			  << "</td>" << endl;
		else if (*it == "end")
			s << "<td class=\""
			  << (t->isPlanEndOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" nowrap>"
			  << time2ISO(t->getPlanEnd())
			  << "</td>" << endl;
		else if (*it == "minstart")
			textTwoRows(time2ISO(t->getMinStart()), r != 0, "");
		else if (*it == "maxstart")
			textTwoRows(time2ISO(t->getMaxStart()), r != 0, "");
		else if (*it == "minend")
			textTwoRows(time2ISO(t->getMinEnd()), r != 0, "");
		else if (*it == "maxend")
			textTwoRows(time2ISO(t->getMaxEnd()), r != 0, "");
		else if (*it == "duration")
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right\""
			  << " nowrap>";
			scaleTime(t->getPlanCalcDuration());
			s << "</td>" << endl;
		}
		else if (*it == "effort")
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right\""
			  << " nowrap>";
			scaleTime(t->getPlanLoad(Interval(start, end), r));
			s << "</td>" << endl;
		}
		else if (*it == "resources")
			planResources(t, r != 0);
		else if (*it == "responsible")
			textTwoRows(htmlFilter(t->getResponsible()->getName()), r != 0,
						"left");
		else if (*it == "responsibilities")
			emptyPlan(r != 0);
		else if (*it == "depends")
			generateDepends(t, r != 0);
		else if (*it == "follows")
			generateFollows(t, r != 0);
		else if (*it == "schedule")
			emptyPlan(r != 0);
		else if (*it == "note")
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" rowspan=\""
			  << (showActual ? "2" : "1")
			  << "\" style=\"text-align:left\">"
			  << "<font size=\"-2\">";
			s << htmlFilter(t->getNote());
			s << "</font></td>" << endl;
		}
		else if (*it == "costs")
			textOneRow(
				QString().sprintf("%5.3f",
								  t->getPlanCosts(Interval(start, end), r)),
				r != 0,
				"right");
		else if (*it == "priority")
			textTwoRows(QString().sprintf("%d", t->getPriority()), r != 0,
						"right");
		else if (*it == "daily")
			dailyTaskPlan(t, r);
		else if (*it == "weekly")
			weeklyTaskPlan(t, r);
		else if (*it == "monthly")
			monthlyTaskPlan(t, r);
		else
			qFatal("generatePlanTask: Unknown Column %s",
				   (*it).latin1());
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generateActualTask(Task* t, Resource* r)
{
	s << "<tr bgcolor=\"" << colAvailable << "\">" << endl;
	for (QStringList::Iterator it = columns.begin();
		 it != columns.end();
		 ++it )
	{
		if (*it == "start")
		{
			s << "<td class=\""
			  << (t->isActualStartOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" nowrap>"
			  << time2ISO(t->getActualStart())
			  << "</td>" << endl;
		}
		else if (*it == "end")
		{
			s << "<td class=\""
			  << (t->isActualEndOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" nowrap>"
			  << time2ISO(t->getActualEnd())
			  << "</td>" << endl;
		}
		else if (*it == "duration")
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right\""
			  << " nowrap>";
			scaleTime(t->getActualCalcDuration());
			s << "</td>" << endl;
		}
		else if (*it == "effort")
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right\""
			  << " nowrap>";
			scaleTime(t->getActualLoad(Interval(start, end), r));
			s << "</td>" << endl;
		}
		else if (*it == "resources")
			actualResources(t, r != 0);
		else if (*it == "costs")
			textOneRow(
				QString().sprintf("%5.3f",
								  t->getActualCosts(Interval(start, end), r)),
				r != 0,
				"right");
		if (*it == "daily")
			dailyTaskActual(t, r);
		else if (*it == "weekly")
			weeklyTaskActual(t, r);
		else if (*it == "monthly")
			monthlyTaskActual(t, r);
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generatePlanResource(Resource* r, Task* t)
{
	s << "<tr valign=\"center\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "no")
			textTwoRows((t == 0 ? QString().sprintf("%d.", r->getIndex()) :
						 QString("")), t != 0, "");
		else if (*it == "id")
			textTwoRows(htmlFilter(r->getId()), t != 0, "left");
		else if (*it == "name")
			resourceName(r, t, FALSE);
		else if (*it == "start")
			emptyPlan(t != 0);
		else if (*it == "end")
			emptyPlan(t != 0);
		else if (*it == "minstart")
			emptyPlan(t != 0);
		else if (*it == "maxstart")
			emptyPlan(t != 0);
		else if (*it == "minend")
			emptyPlan(t != 0);
		else if (*it == "maxend")
			emptyPlan(t != 0);
		else if (*it == "duration")
			emptyPlan(t != 0);
		else if (*it == "effort")
		{
			s << "<td class=\""
			  << (t == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right\""
			  << " nowrap>";
			scaleTime(r->getPlanLoad(Interval(start, end), t));
			s << "</td>" << endl;
		}
		else if (*it == "resources")
			emptyPlan(t != 0);
		else if (*it == "responsible")
			emptyPlan(t != 0);
		else if (*it == "responsibilities")
			generateResponsibilities(r, t != 0);
		else if (*it == "depends")
			emptyPlan(t != 0);
		else if (*it == "follows")
			emptyPlan(t != 0);
		else if (*it == "schedule")
		{
			if (t == 0)
				emptyPlan(FALSE);
			else
				planSchedule(r, t);
		}
		else if (*it == "note")
			emptyPlan(t != 0);
		else if (*it == "costs")
			textOneRow(
				QString().sprintf("%5.3f",
								  r->getPlanCosts(Interval(start, end), t)),
				t != 0,
				"right");
		else if (*it == "priority")
			emptyPlan(t != 0);
		else if (*it == "daily")
			dailyResourcePlan(r, t);
		else if (*it == "weekly")
			weeklyResourcePlan(r, t);
		else if (*it == "monthly")
			monthlyResourcePlan(r, t);
		else
			qFatal("generatePlanResource: Unknown Column %s",
				   (*it).latin1());
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generateActualResource(Resource* r, Task* t)
{
	s << "<tr valign=\"center\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "effort")
		{
			s << "<td class=\""
			  << (t == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right\""
			  << " nowrap>";
			scaleTime(r->getActualLoad(Interval(start, end), t));
			s << "</td>" << endl;
		}
		else if (*it == "schedule")
		{
			if (t != 0)
				actualSchedule(r, t);
		}
		else if (*it == "costs")
			textOneRow(
				QString().sprintf("%5.3f",
								  r->getActualCosts(Interval(start, end), t)),
				t != 0,
				"right");
		else if (*it == "daily")
			dailyResourceActual(r, t);
		else if (*it == "weekly")
			weeklyResourceActual(r, t);
		else if (*it == "monthly")
			monthlyResourceActual(r, t);
	}
	s << "</tr>" << endl;
}

void
ReportHtml::reportHTMLHeader()
{
	s << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">" << endl
	  << "<! Generated by TaskJuggler v"VERSION">"
	  << "<html>" << endl
	  << "<head>" << endl
	  << "<title>Task Report</title>" << endl
	  << "<style type=\"text/css\"><!--" << endl;
	s.reset();
	s.setf(QTextStream::hex);
	s << ".default { background-color:#" << colDefault
	  << "; font-size:70%; text-align:center }" << endl
	  << ".defaultlight { background-color:#" << colDefaultLight
	  << "; font-size:70%; text-align:center }" << endl
	  << ".task { background-color:#" << colDefault
	  << "; font-size:100%; text-align:left }" << endl
	  << ".tasklight { background-color:#" << colDefaultLight
	  << "; font-size:100%; text-align:left }" << endl
	  << ".available { background-color:#" << colAvailable
	  << "; font-size:70%; text-align:center }" << endl
	  << ".vacation { background-color:#" << colVacation
	  << "; font-size:70%; text-align:center }" << endl
	  << ".weekend { background-color:#" << colWeekend
	  << "; font-size:70%; text-align:center }" << endl
	  << ".milestone { background-color:#" << colMilestone
	  << "; font-size:70%; text-align:center }" << endl
	  << ".booked { background-color:#" << colBooked
	  << "; font-size:70%; text-align:center }" << endl
	  << ".bookedlight { background-color:#" << colBookedLight
	  << "; font-size:70%; text-align:center }" << endl
	  << ".headersmall { background-color:#" << colHeader
	  << "; font-size:70%; text-align:center }" << endl
	  << ".headerbig { background-color:#" << colHeader
	  << "; font-size:110%; font-weight:bold; text-align:center }" << endl
	  << ".completed { background-color:#" << colCompleted
	  << "; font-size:70%; text-align:center }" << endl
	  << ".completedlight { background-color:#" << colCompletedLight
	  << "; font-size:70%; text-align:center }" << endl
	  << ".today { background-color:#" << colToday
	  << "; font-size:70%; text-align:center }" << endl
	  << "--></style>" << endl
	  << "</head>" << endl
	  << "<body>" << endl;
}

void
ReportHtml::reportHTMLFooter()
{
	s << "<p><font size=\"-2\">"
	  << project->getCopyright()
	  << " - " << project->getVersion()
	  << " - TaskJuggler v"
	  << VERSION << "</font></p></body>\n";
}

bool
ReportHtml::generateTableHeader()
{
	// Header line 1
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "no")
			s << "<td class=\"headerbig\" rowspan=\"2\">No.</td>";
		else if (*it == "id")
			s << "<td class=\"headerbig\" rowspan=\"2\">ID</td>";
		else if (*it == "name")
			s << "<td class=\"headerbig\" rowspan=\"2\">Name</td>";
		else if (*it == "start")
			s << "<td class=\"headerbig\" rowspan=\"2\">Start</td>";
		else if (*it == "end")
			s << "<td class=\"headerbig\" rowspan=\"2\">End</td>";
		else if (*it == "minstart")
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. Start</td>";
		else if (*it == "maxstart")
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. Start</td>";
		else if (*it == "minend")
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. End</td>";
		else if (*it == "maxend")
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. End</td>";
		else if (*it == "duration")
			s << "<td class=\"headerbig\" rowspan=\"2\">Duration</td>";
		else if (*it == "effort")
			s << "<td class=\"headerbig\" rowspan=\"2\">Effort</td>";
		else if (*it == "resources")
			s << "<td class=\"headerbig\" rowspan=\"2\">Resources</td>";
		else if (*it == "responsible")
			s << "<td class=\"headerbig\" rowspan=\"2\">Responsible</td>";
		else if (*it == "responsibilities")
			s << "<td class=\"headerbig\" rowspan=\"2\">Responsibilities</td>";
		else if (*it == "depends")
			s << "<td class=\"headerbig\" rowspan=\"2\">Dependencies</td>";
		else if (*it == "follows")
			s << "<td class=\"headerbig\" rowspan=\"2\">Followers</td>";
		else if (*it == "schedule")
			s << "<td class=\"headerbig\" rowspan=\"2\">Schedule</td>";
		else if (*it == "note")
			s << "<td class=\"headerbig\" rowspan=\"2\">Note</td>";
		else if (*it == "costs")
			s << "<td class=\"headerbig\" rowspan=\"2\">Costs</td>";
		else if (*it == "priority")
			s << "<td class=\"headerbig\" rowspan=\"2\">Priority</td>";
		else if (*it == "daily")
			htmlDayHeaderMonths();
		else if (*it == "weekly")
			htmlWeekHeaderMonths();
		else if (*it == "monthly")
			htmlMonthHeaderYears();
		else
		{
			qWarning("Unknown Column '%s' for HTML Task Report\n",
					(*it).latin1());
			return FALSE;
		}
	}
	s << "</tr>" << endl;

	// Header line 2
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "daily")
			htmlDayHeaderDays();
		else if (*it == "weekly")
			htmlWeekHeaderWeeks();
		else if (*it == "monthly")
			htmlMonthHeaderMonths();
	}
	s << "</tr>\n" << endl;

	return TRUE;
}

void
ReportHtml::htmlDayHeaderDays(bool highlightNow)
{
	for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
	{
		int dom = dayOfMonth(day);
		s << "<td class=\"" <<
			(highlightNow && isSameDay(project->getNow(), day) ?
			 "today" : isWeekend(day) ? "weekend" : "headersmall")
		  << "\"><font size=\"-2\">&nbsp;";
		if (dom < 10)
			s << "&nbsp;";
		s << QString().sprintf("%d", dom) << "</font></td>";
	}
}

void
ReportHtml::htmlDayHeaderMonths()
{
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t day = midnight(start); day < end;
		 day = beginOfMonth(sameTimeNextMonth(day)))
	{
		int left = daysLeftInMonth(day);
		if (left > daysBetween(day, end))
			left = daysBetween(day, end);
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left)
		  << "\">"
		  << monthAndYear(day) << "</td>" << endl;
	}
}

void
ReportHtml::htmlWeekHeaderWeeks(bool highlightNow)
{
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		int woy = weekOfYear(week);
		s << "<td class=\"" <<
			(highlightNow && isSameWeek(project->getNow(), week) ?
			 "today" : "headersmall")
		  << "\"><font size=\"-2\">&nbsp;";
		if (woy < 10)
			s << "&nbsp;";
		s << QString().sprintf("%d", woy) << "</font></td>";
	}
}

void
ReportHtml::htmlWeekHeaderMonths()
{
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t week = beginOfWeek(start); week < end; )
	{
		int left = weeksLeftInMonth(week);
		if (left > weeksBetween(week, end))
			left = weeksBetween(week, end);
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left)
		  << "\">"
		  << monthAndYear(week) << "</td>" << endl;

		time_t newWeek = beginOfWeek(beginOfMonth(sameTimeNextMonth(week)));
		if (isSameMonth(newWeek, week))
			week = sameTimeNextWeek(newWeek);
		else
			week = newWeek;
	}
}

void
ReportHtml::htmlMonthHeaderMonths(bool highlightNow)
{
	static char* mnames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
							  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		int moy = monthOfYear(month);
		s << "<td class=\"" <<
			(highlightNow && isSameMonth(project->getNow(), month) ?
			 "today" : "headersmall")
		  << "\"><font size=\"-2\">&nbsp;";
		s << mnames[moy - 1] << "</font></td>";
	}
}

void
ReportHtml::htmlMonthHeaderYears()
{
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t year = midnight(start); year < end;
		 year = beginOfYear(sameTimeNextYear(year)))
	{
		int left = monthLeftInYear(year);
		if (left > monthsBetween(year, end))
			left = monthsBetween(year, end);
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left) << "\">"
		  << QString().sprintf("%d", ::year(year)) << "</td>" << endl;
	}
}

void
ReportHtml::emptyPlan(bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\">&nbsp;</td>";
}

void
ReportHtml::emptyActual(bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\">&nbsp;</td>";
}

void
ReportHtml::textOneRow(const QString& text, bool light, const QString& align)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default") << "\"";
	if (!align.isEmpty())
		s << " style=\"text-align:" << align << "\"";
	s << " nowrap>"
	  << text << "</td>";
}

void
ReportHtml::textTwoRows(const QString& text, bool light, const QString& align)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1") << "\"";
	if (!align.isEmpty())
		s << " style=\"text-align:" << align << "\"";
	s << " nowrap>"
	  << text << "</td>";
}

void
ReportHtml::dailyResourcePlan(Resource* r, Task* t)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getPlanLoad(Interval(day).firstDay(), t);
		QString bgCol = 
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedLight") :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) || r->hasVacationDay(day) ?
			"vacation" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::dailyResourceActual(Resource* r, Task* t)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getActualLoad(Interval(day).firstDay(), t);
		QString bgCol = 
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(sameTimeNextDay(day) - 1) ?
			  "completedlight" : "bookedLight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) || r->hasVacationDay(day) ?
			"vacation" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void 
ReportHtml::dailyTaskPlan(Task* t, Resource* r)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getPlanLoad(Interval(day).firstDay(), r);
		QString bgCol = 
			t->isPlanActive(Interval(day).firstDay()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 ? "booked" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) ? "vacation" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::dailyTaskActual(Task* t, Resource* r)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getActualLoad(Interval(day).firstDay(), r);
		QString bgCol = 
			t->isActualActive(Interval(day).firstDay()) ? 
			(t->isCompleted(sameTimeNextDay(day) - 1) ?
			 (r == 0 ? "completed" : "completedlight") :
			 t->isMilestone() ? "milestone" :
			 (r == 0 ? "booked" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) ? "vacation" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::weeklyResourcePlan(Resource* r, Task* t)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = r->getPlanLoad(Interval(week).firstWeek(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedlight") :
			isSameWeek(project->getNow(), week) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::weeklyResourceActual(Resource* r, Task* t)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = r->getActualLoad(Interval(week).firstWeek(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(sameTimeNextWeek(week) - 1) ?
			  "completedlight" : "bookedLight")) :
			isSameWeek(project->getNow(), week) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void 
ReportHtml::weeklyTaskPlan(Task* t, Resource* r)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getPlanLoad(Interval(week).firstWeek(), r);
		QString bgCol = 
			t->isPlanActive(Interval(week).firstWeek()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 ? "booked" : "bookedlight")) :
			isSameWeek(project->getNow(), week) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::weeklyTaskActual(Task* t, Resource* r)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getActualLoad(Interval(week).firstWeek(), r);
		QString bgCol = 
			t->isActualActive(Interval(week).firstWeek()) ?
			(t->isCompleted(sameTimeNextWeek(week) - 1) ?
			 (r == 0 ? "completed" : "completedlight") :
			 t->isMilestone() ? "milestone" :
			 (r == 0 ? "booked" : "bookedlight")) :
			isSameWeek(project->getNow(), week) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::monthlyResourcePlan(Resource* r, Task* t)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getPlanLoad(Interval(month).firstMonth(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedlight") :
			isSameMonth(project->getNow(), month) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::monthlyResourceActual(Resource* r, Task* t)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getActualLoad(Interval(month).firstMonth(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(sameTimeNextMonth(month) - 1) ?
			  "completedlight" : "bookedLight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::monthlyTaskPlan(Task* t, Resource* r)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = t->getPlanLoad(Interval(month).firstMonth(), r);
		QString bgCol = 
			t->isPlanActive(Interval(month).firstMonth()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 ? "booked" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::monthlyTaskActual(Task* t, Resource* r)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = t->getActualLoad(Interval(month).firstMonth(), r);
		QString bgCol = 
			t->isActualActive(Interval(month).firstMonth()) ?
			(t->isCompleted(sameTimeNextMonth(month) - 1) ?
			 (r == 0 ? "completed" : "completedlight"):
			 t->isMilestone() ? "milestone" :
			 (r == 0 ? "booked" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::taskName(Task* t, Resource* r, bool big)
{
	QString spaces = "";
	int fontSize = big ? 2 : -1;
	if (resourceSortCriteria == CoreAttributesList::TreeMode)
		for (Resource* rp = r ; rp != 0; --fontSize)
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			rp = rp->getParent();
		}

	if (taskSortCriteria == CoreAttributesList::TreeMode)
	{
		Task* tp = t->getParent();
		for ( ; tp != 0; --fontSize)
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			tp = tp->getParent();
		}
		s << "<td class=\""
		  << (r == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" nowrap>"
		  << spaces
		  << "<font size=\""
		  << (fontSize < 0 ? '-' : '+') 
		  << (fontSize < 0 ? -fontSize : fontSize) << "\">"
		  << htmlFilter(t->getName())
		  << "</font></td>" << endl;
	}
	else
		s << "<td class=\""
		  << (r == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" nowrap>"
		  << spaces << htmlFilter(t->getName())
		  << "</td>" << endl;
}

void
ReportHtml::resourceName(Resource* r, Task* t, bool big)
{
	QString spaces = "";
	int fontSize = big ? 2 : -1;
	if (taskSortCriteria == CoreAttributesList::TreeMode)
		for (Task* tp = t; tp != 0; --fontSize)
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			tp = tp->getParent();
		}

	if (resourceSortCriteria == CoreAttributesList::TreeMode)
	{
		Resource* rp = r->getParent();
		for ( ; rp != 0; --fontSize)
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			rp = rp->getParent();
		}
		s << "<td class=\""
		  << (t == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" nowrap>"
		  << spaces
		  << "<font size=\""
		  << (fontSize < 0 ? '-' : '+') 
		  << (fontSize < 0 ? -fontSize : fontSize) << "\">"
		  << htmlFilter(r->getName())
		  << "</font></td>" << endl;
	}
	else
		s << "<td class=\""
		  << (t == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" nowrap>"
		  << spaces << htmlFilter(r->getName())
		  << "</td>" << endl;
}

void
ReportHtml::planResources(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" style=\"text-align:left\">"
	  << "<font size=\"-2\">";
	bool first = TRUE;
	QPtrList<Resource> planResources = t->getPlanBookedResources();
	for (Resource* r = planResources.first(); r != 0;
		 r = planResources.next())
	{
		if (!first)
			s << ", ";
					
		s << htmlFilter(r->getName());
		first = FALSE;
	}
	s << "</font></td>" << endl;
}

void
ReportHtml::actualResources(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" style=\"text-align:left\">"
	  << "<font size=\"-2\">";
	bool first = TRUE;
	QPtrList<Resource> actualResources = t->getActualBookedResources();
	for (Resource* r = actualResources.first(); r != 0;
		 r = actualResources.next())
	{
		if (!first)
			s << ", ";
					
		s << htmlFilter(r->getName());
		first = FALSE;
	}
	s << "</font></td>" << endl;
}

void
ReportHtml::generateDepends(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\"><font size=\"-2\">";
	bool first = TRUE;
	for (Task* d = t->firstPrevious(); d != 0;
		 d = t->nextPrevious())
	{
		if (!first)
			s << ", ";
		s << QString().sprintf("%d", d->getIndex());
	}
	s << "</font></td>" << endl;
}

void
ReportHtml::generateFollows(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<font size=\"-2\">";
	bool first = TRUE;
	for (Task* d = t->firstFollower(); d != 0;
		 d = t->nextFollower())
	{
		if (!first)
			s << ", ";
		s << QString().sprintf("%d", d->getIndex());
		first = FALSE;
	}
	s << "</font></td>" << endl;
}

void
ReportHtml::generateResponsibilities(Resource*r, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<font size=\"-2\">";
	bool first = TRUE;
	TaskList tl = project->getTaskList();
	for (Task* t = tl.first(); t != 0; t = tl.next())
	{
		if (t->getResponsible() == r)
		{
			if (!first)
				s << ", ";
			s << QString().sprintf("%d", t->getIndex());
			first = FALSE;
		}
	}
	s << "</font></td>" << endl;
}

void
ReportHtml::planSchedule(Resource* r, Task* t)
{
	s << "<td class=\""
	  << (t == 0 ? "default" : "defaultlight") 
	  << "\" style=\"text-align:left\">";

	bool first = TRUE;
	BookingList planJobs = r->getPlanJobs();
	for (Booking* b = planJobs.first(); b != 0; b = planJobs.next())
	{
		if (t == b->getTask())
		{
			if (!first)
				s << ", ";
			else
				first = FALSE;
			s << time2ISO(b->getStart()) << " - "
			  << time2ISO(b->getEnd()) << endl;
		}
	}

	s << "</td>" << endl;
}

void
ReportHtml::actualSchedule(Resource* r, Task* t)
{
	s << "<td class=\""
	  << (t == 0 ? "default" : "defaultlight") 
	  << "\" style=\"text-align:left\">";

	bool first = TRUE;
	BookingList actualJobs = r->getActualJobs();
	for (Booking* b = actualJobs.first(); b != 0; b = actualJobs.next())
	{
		if (t == b->getTask())
		{
			if (!first)
				s << ", ";
			else
				first = FALSE;
			s << time2ISO(b->getStart()) << " - "
			  << time2ISO(b->getEnd()) << endl;
		}
	}

	s << "</td>" << endl;
}

QString
ReportHtml::htmlFilter(const QString& s)
{
	QString out;
	bool parTags = FALSE;
	for (uint i = 0; i < s.length(); i++)
	{
		QString repl;
		if (s[i] == '&')
			repl = "&amp;";
		else if (s[i] == '<')
			repl = "&lt;";
		else if (s[i] == '>')
			repl = "&gt;";
		else if (s.mid(i, 2) == "\n\n")
		{
			repl = "</p><p>";
			parTags = TRUE;
			i++;
		}

		if (repl.isEmpty())
			out += s[i];
		else
			out += repl;
	}

	return parTags ? QString("<p>") + out + "</p>" : out;
}

void
ReportHtml::reportLoad(double load, const QString& bgCol, bool bold)
{
	if (load > 0.0)
	{
		s << "<td class=\""
		  << bgCol << "\">";
		if (bold)
			s << "<b>";
		s << QString().sprintf("%3.1f", load);
//		scaleTime(load, FALSE);
		if (bold)
			s << "</b>";
		s << "</td>" << endl;
	}
	else
		s << "<td class=\""
		  << bgCol << "\">&nbsp;</td>" << endl;
}
