/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include "Project.h"
#include "Report.h"
#include "Utility.h"
#include "MacroTable.h"

#define KW(a) a

ReportHtml::ReportHtml(Project* p, const QString& f, time_t s, time_t e,
					   const QString& df, int dl) :
   Report(p, f, s, e, df, dl)
{
	colDefault = 0xf3ebae;
	colDefaultLight = 0xfffadd;
	colWeekend = 0xffec80;
	colVacation = 0xfffc60;
	colAvailable = 0xa4ff8d;
	colBooked = 0xff5a5d;
	colBookedLight = 0xffbfbf;
	colHeader = 0xa5c2ff;
	colMilestone = 0xff2a2a;
	colCompleted = 0x87ff75;
	colCompletedLight = 0xa1ff9a;
	colToday = 0xa387ff;

	registerUrl(KW("dayheader"));
	registerUrl(KW("monthheader"));
	registerUrl(KW("resourcename"));
	registerUrl(KW("taskname"));
	registerUrl(KW("weekheader"));
	registerUrl(KW("yearheader"));
}

void
ReportHtml::generatePlanTask(Task* t, Resource* r)
{
	s << "<tr valign=\"middle\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("no"))
			textTwoRows((r == 0 ? QString().sprintf("%d.", t->getIndex()) :
						 QString("")), r != 0, "");
		else if (*it == KW("id"))
			textTwoRows(htmlFilter(t->getId()), r != 0, "left");
		else if (*it == KW("name"))
			taskName(t, r, r == 0);
		else if (*it == KW("start"))
			s << "<td class=\""
			  << (t->isPlanStartOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"text-align:left white-space:nowrap\">"
			  << time2ISO(t->getPlanStart())
			  << "</td>" << endl;
		else if (*it == KW("end"))
			s << "<td class=\""
			  << (t->isPlanEndOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"text-align:left white-space:nowrap\">"
			  << time2ISO(t->getPlanEnd())
			  << "</td>" << endl;
		else if (*it == KW("minstart"))
			textTwoRows(time2ISO(t->getMinStart()), r != 0, "");
		else if (*it == KW("maxstart"))
			textTwoRows(time2ISO(t->getMaxStart()), r != 0, "");
		else if (*it == KW("minend"))
			textTwoRows(time2ISO(t->getMinEnd()), r != 0, "");
		else if (*it == KW("maxend"))
			textTwoRows(time2ISO(t->getMaxEnd()), r != 0, "");
		else if (*it == KW("startbuffer"))
			textTwoRows(QString().sprintf("%3.0f", t->getStartBuffer()),
						r != 0, "right");
		else if (*it == KW("endbuffer"))
			textTwoRows(QString().sprintf("%3.0f", t->getEndBuffer()),
						r != 0, "right");
		else if (*it == KW("startbufferend"))
			textOneRow(time2ISO(t->getPlanStartBufferEnd()), r != 0, "left");
		else if (*it == KW("endbufferstart"))
			textOneRow(time2ISO(t->getPlanEndBufferStart()), r != 0, "left");
		else if (*it == KW("duration"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getPlanCalcDuration())
			  << "</td>" << endl;
		}
		else if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getPlanLoad(Interval(start, end), r))
			  << "</td>" << endl;
		}
		else if (*it == KW("projectid"))
			textTwoRows(t->getProjectId() + " (" +
						project->getIdIndex(t->getProjectId()) + ")", r != 0,
						"left");
		else if (*it == KW("resources"))
			planResources(t, r != 0);
		else if (*it == KW("responsible"))
			textTwoRows(htmlFilter(t->getResponsible()->getName()), r != 0,
						"left");
		else if (*it == KW("responsibilities"))
			emptyPlan(r != 0);
		else if (*it == KW("depends"))
			generateDepends(t, r != 0);
		else if (*it == KW("follows"))
			generateFollows(t, r != 0);
		else if (*it == KW("schedule"))
			planSchedule(r, t);
		else if (*it == KW("mineffort"))
			emptyPlan(r != 0);
		else if (*it == KW("maxeffort"))
			emptyPlan(r != 0);
		else if (*it == KW("rate"))
			emptyPlan(r != 0);
		else if (*it == KW("kotrusid"))
			emptyPlan(r != 0);
		else if (*it == KW("note"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" rowspan=\""
			  << (showActual ? "2" : "1")
			  << "\" style=\"text-align:left\">"
			  << "<span style=\"font-size:0.8em\">";
			s << htmlFilter(t->getNote());
			s << "</span></td>" << endl;
		}
		else if (*it == KW("costs"))
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  t->getPlanCredits(Interval(start, end), r)),
				r != 0,
				"right");
		else if (*it == KW("priority"))
			textTwoRows(QString().sprintf("%d", t->getPriority()), r != 0,
						"right");
		else if (*it == KW("daily"))
			dailyTaskPlan(t, r);
		else if (*it == KW("weekly"))
			weeklyTaskPlan(t, r);
		else if (*it == KW("monthly"))
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
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin();
		 it != columns.end();
		 ++it )
	{
		if (*it == KW("start"))
		{
			s << "<td class=\""
			  << (t->isActualStartOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"text-align:left white-space:nowrap\">"
			  << time2ISO(t->getActualStart())
			  << "</td>" << endl;
		}
		else if (*it == KW("end"))
		{
			s << "<td class=\""
			  << (t->isActualEndOk() ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"white-space:nowrap\">"
			  << time2ISO(t->getActualEnd())
			  << "</td>" << endl;
		}
		else if (*it == KW("startbufferend"))
			textOneRow(time2ISO(t->getActualStartBufferEnd()), r != 0, "left");
		else if (*it == KW("endbufferstart"))
			textOneRow(time2ISO(t->getActualEndBufferStart()), r != 0, "left");
		else if (*it == KW("duration"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getActualCalcDuration())
			  << "</td>" << endl;
		}
		else if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getActualLoad(Interval(start, end), r))
			  << "</td>" << endl;
		}
		else if (*it == KW("resources"))
			actualResources(t, r != 0);
		else if (*it == "costs")
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  t->getActualCredits(Interval(start, end),
													  r)),
				r != 0,
				"right");
		if (*it == KW("daily"))
			dailyTaskActual(t, r);
		else if (*it == KW("weekly"))
			weeklyTaskActual(t, r);
		else if (*it == KW("monthly"))
			monthlyTaskActual(t, r);
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generatePlanResource(Resource* r, Task* t)
{
	s << "<tr valign=\"middle\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("no"))
			textTwoRows((t == 0 ? QString().sprintf("%d.", r->getIndex()) :
						 QString("")), t != 0, "");
		else if (*it == KW("id"))
			textTwoRows(htmlFilter(r->getId()), t != 0, "left");
		else if (*it == KW("name"))
			resourceName(r, t, FALSE);
		else if (*it == KW("start"))
			emptyPlan(t != 0);
		else if (*it == KW("end"))
			emptyPlan(t != 0);
		else if (*it == KW("minstart"))
			emptyPlan(t != 0);
		else if (*it == KW("maxstart"))
			emptyPlan(t != 0);
		else if (*it == KW("minend"))
			emptyPlan(t != 0);
		else if (*it == KW("maxend"))
			emptyPlan(t != 0);
		else if (*it == KW("startbuffer"))
			emptyPlan(t != 0);
		else if (*it == KW("endbuffer"))
			emptyPlan(t != 0);
		else if (*it == KW("startbufferend"))
			emptyPlan(t != 0);
		else if (*it == KW("endbufferstart"))
			emptyPlan(t != 0);
		else if (*it == KW("duration"))
			emptyPlan(t != 0);
		else if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (t == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(r->getPlanLoad(Interval(start, end), t))
			  << "</td>" << endl;
		}
		else if (*it == KW("projectid"))
			emptyPlan(t != 0);
		else if (*it == KW("resources"))
			emptyPlan(t != 0);
		else if (*it == KW("responsible"))
			emptyPlan(t != 0);
		else if (*it == KW("responsibilities"))
			generateResponsibilities(r, t != 0);
		else if (*it == KW("depends"))
			emptyPlan(t != 0);
		else if (*it == KW("follows"))
			emptyPlan(t != 0);
		else if (*it == KW("schedule"))
				planSchedule(r, t);
		else if (*it == KW("mineffort"))
			textTwoRows(QString().sprintf("%.2f", r->getMinEffort()), t != 0,
						"right");
		else if (*it == KW("maxeffort"))
			textTwoRows(QString().sprintf("%.2f", r->getMaxEffort()), t != 0,
						"right");
		else if (*it == KW("rate"))
			textTwoRows(QString().sprintf("%.*f", project->getCurrencyDigits(),
										  r->getRate()), t != 0,
						"right");
		else if (*it == KW("kotrusid"))
			textTwoRows(r->getKotrusId(), t != 0, "left");
		else if (*it == KW("note"))
			emptyPlan(t != 0);
		else if (*it == KW("costs"))
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  r->getPlanCredits(Interval(start, end), t)),
				t != 0,
				"right");
		else if (*it == KW("priority"))
			emptyPlan(t != 0);
		else if (*it == KW("daily"))
			dailyResourcePlan(r, t);
		else if (*it == KW("weekly"))
			weeklyResourcePlan(r, t);
		else if (*it == KW("monthly"))
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
	s << "<tr valign=\"middle\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (t == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(r->getActualLoad(Interval(start, end), t))
			  << "</td>" << endl;
		}
		else if (*it == KW("schedule"))
		{
			if (t != 0)
				actualSchedule(r, t);
		}
		else if (*it == KW("costs"))
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  r->getActualCredits(Interval(start, end),
													  t)),
				t != 0,
				"right");
		else if (*it == KW("daily"))
			dailyResourceActual(r, t);
		else if (*it == KW("weekly"))
			weeklyResourceActual(r, t);
		else if (*it == KW("monthly"))
			monthlyResourceActual(r, t);
	}
	s << "</tr>" << endl;
}

void
ReportHtml::reportHTMLHeader()
{
	s << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">" << endl
	  << "<!-- Generated by TaskJuggler v"VERSION" -->" << endl
	  << "<!-- For details about TaskJuggler see "
	  << "http://www.suse.de/~freitag/taskjuggler -->" << endl
	  << "<html>" << endl
	  << "<head>" << endl
	  << "<title>Task Report</title>" << endl
	  << "<style type=\"text/css\"><!--" << endl;
	if (rawStyleSheet.isEmpty())
	{
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
			<< "; font-size:70%; text-align:center }" << endl;
	}
	else
		s << rawStyleSheet << endl;
	s << "--></style>" << endl;
	s << "</head>" << endl
	  << "<body>" << endl;

	if (!headline.isEmpty())
		s << "<h1>" << htmlFilter(headline) << "</h1>" << endl;
	if (!caption.isEmpty())
		s << "<p>" << htmlFilter(caption) << "<p>" << endl;
	if (!rawHead.isEmpty())
		s << rawHead << endl;
}

void
ReportHtml::reportHTMLFooter()
{
	if (!rawTail.isEmpty())
		s << rawTail << endl;

	s << "<p><span style=\"font-size:0.6em\">";
	if (project->getCopyright() != "")
		s << htmlFilter(project->getCopyright()) << " - ";
	s << htmlFilter(project->getVersion())
	  << " - TaskJuggler v"
	  << VERSION << "</span></p></body>\n";
}

bool
ReportHtml::generateTableHeader()
{
	// Header line 1
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("no"))
			s << "<td class=\"headerbig\" rowspan=\"2\">No.</td>";
		else if (*it == KW("id"))
			s << "<td class=\"headerbig\" rowspan=\"2\">ID</td>";
		else if (*it == KW("name"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Name</td>";
		else if (*it == KW("start"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Start</td>";
		else if (*it == KW("end"))
			s << "<td class=\"headerbig\" rowspan=\"2\">End</td>";
		else if (*it == KW("minstart"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. Start</td>";
		else if (*it == KW("maxstart"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. Start</td>";
		else if (*it == KW("minend"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. End</td>";
		else if (*it == KW("maxend"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. End</td>";
		else if (*it == KW("startbufferend"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Start Buf. End</td>";
		else if (*it == KW("endbufferstart"))
			s << "<td class=\"headerbig\" rowspan=\"2\">End Buf. Start</td>";
		else if (*it == KW("startbuffer"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Start Buf.</td>";
		else if (*it == KW("endbuffer"))
			s << "<td class=\"headerbig\" rowspan=\"2\">End Buf.</td>";
		else if (*it == KW("duration"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Duration</td>";
		else if (*it == KW("effort"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Effort</td>";
		else if (*it == KW("projectid"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Project ID</td>";
		else if (*it == KW("resources"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Resources</td>";
		else if (*it == KW("responsible"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Responsible</td>";
		else if (*it == KW("responsibilities"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Responsibilities</td>";
		else if (*it == KW("depends"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Dependencies</td>";
		else if (*it == KW("follows"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Followers</td>";
		else if (*it == KW("schedule"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Schedule</td>";
		else if (*it == KW("mineffort"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. Effort</td>";
		else if (*it == KW("maxeffort"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. Effort</td>";
		else if (*it == KW("rate"))
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">Rate";
			if (!project->getCurrency().isEmpty())
				s << " " << htmlFilter(project->getCurrency());
			s << "</td>";
		}
		else if (*it == KW("kotrusid"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Kotrus ID</td>";
		else if (*it == KW("note"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Note</td>";
		else if (*it == KW("costs"))
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">Costs";
			if (!project->getCurrency().isEmpty())
				s << " " << htmlFilter(project->getCurrency());
			s << "</td>";
		}
		else if (*it == KW("priority"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Priority</td>";
		else if (*it == KW("daily"))
			htmlDailyHeaderMonths();
		else if (*it == KW("weekly"))
			htmlWeeklyHeaderMonths();
		else if (*it == KW("monthly"))
			htmlMonthlyHeaderYears();
		else
		{
			qWarning("Unknown Column '%s' for HTML Task Report\n",
					(*it).latin1());
			return FALSE;
		}
	}
	s << "</tr>" << endl;

	// Header line 2
	bool td = FALSE;
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("daily"))
		{
			td = TRUE;
			htmlDailyHeaderDays();
		}
		else if (*it == KW("weekly"))
		{
			td = TRUE;
			htmlWeeklyHeaderWeeks();
		}
		else if (*it == KW("monthly"))
		{
			td = TRUE;
			htmlMonthlyHeaderMonths();
		}
	}
	if (!td)
		s << "<td>&nbsp;</td>";
	s << "</tr>\n" << endl;

	return TRUE;
}

void
ReportHtml::htmlDailyHeaderDays(bool highlightNow)
{
	// Generates the 2nd header line for daily calendar views.
	for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
	{
		int dom = dayOfMonth(day);
		s << "<td class=\"" <<
			(highlightNow && isSameDay(project->getNow(), day) ?
			 "today" : isWeekend(day) ? "weekend" : "headersmall")
		  << "\"><span style=\"font-size:0.8em\">&nbsp;";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d", dom),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(day)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(day)),
							  defFileName, defFileLine));
		if (dom < 10)
			s << "&nbsp;";
		s << generateUrl(KW("dayheader"), QString().sprintf("%d", dom));
		s << "</span></td>";
	}
}

void
ReportHtml::htmlDailyHeaderMonths()
{
	// Generates the 1st header line for daily calendar views.
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t day = midnight(start); day < end;
		 day = beginOfMonth(sameTimeNextMonth(day)))
	{
		int left = daysLeftInMonth(day);
		if (left > daysBetween(day, end))
			left = daysBetween(day, end);
		s << "<td class=\"headerbig\" colspan=\""
			<< QString().sprintf("%d", left) << "\">"; 
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(day)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(day)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(day)),
							  defFileName, defFileLine));
		s << generateUrl(KW("monthheader"), monthAndYear(day)); 
		s << "</td>" << endl;
	}
}

void
ReportHtml::htmlWeeklyHeaderWeeks(bool highlightNow)
{
	// Generates the 2nd header line for weekly calendar views.
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		int woy = weekOfYear(week);
		s << "<td class=\"" <<
			(highlightNow && isSameWeek(project->getNow(), week) ?
			 "today" : "headersmall")
		  << "\"><span style=\"font-size:0.8em\">&nbsp;";
		if (woy < 10)
			s << "&nbsp;";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(woy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(woy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(woy)),
							  defFileName, defFileLine));
		s << generateUrl(KW("weekheader"), QString().sprintf("%d", woy));
		s << "</span></td>";
	}
}

void
ReportHtml::htmlWeeklyHeaderMonths()
{
	// Generates the 1st header line for weekly calendar views.
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t week = beginOfWeek(start); week < end; )
	{
		int left = weeksLeftInMonth(week);
		if (left > weeksBetween(week, end))
			left = weeksBetween(week, end);
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left) << "\">";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(week)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(week)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(week)),
							  defFileName, defFileLine));
		s << generateUrl(KW("monthheader"), monthAndYear(week));
		s << "</td>" << endl;

		time_t newWeek = beginOfWeek(beginOfMonth(sameTimeNextMonth(week)));
		if (isSameMonth(newWeek, week))
			week = sameTimeNextWeek(newWeek);
		else
			week = newWeek;
	}
}

void
ReportHtml::htmlMonthlyHeaderMonths(bool highlightNow)
{
	// Generates 2nd header line of monthly calendar view.
	static char* mnames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
							  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		int moy = monthOfYear(month);
		s << "<td class=\"" <<
			(highlightNow && isSameMonth(project->getNow(), month) ?
			 "today" : "headersmall")
		  << "\"><span style=\"font-size:0.8em\">&nbsp;";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(moy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(moy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(moy)),
							  defFileName, defFileLine));
		s << generateUrl(KW("monthheader"), mnames[moy - 1]);
		s << "</span></td>";
	}
}

void
ReportHtml::htmlMonthlyHeaderYears()
{
	// Generates 1st header line of monthly calendar view.
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t year = midnight(start); year < end;
		 year = beginOfYear(sameTimeNextYear(year)))
	{
		int left = monthLeftInYear(year);
		if (left > monthsBetween(year, end))
			left = monthsBetween(year, end);
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left) << "\">";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(year)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(year)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", ::year(year)),
							  defFileName, defFileLine));
		s << generateUrl(KW("yearheader"),
						 QString().sprintf("%d", ::year(year)));
		s << "</td>" << endl;
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
		s << " style=\"text-align:" << align << " white-space:nowrap\"";
	s << ">" << text << "</td>";
}

void
ReportHtml::textTwoRows(const QString& text, bool light, const QString& align)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1") << "\"";
	if (!align.isEmpty())
		s << " style=\"text-align:" << align << " white-space:nowrap\"";
	s << ">" << text << "</td>";
}

void
ReportHtml::dailyResourcePlan(Resource* r, Task* t)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getPlanLoad(Interval(day).firstDay(), t);
		QString bgCol = 
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedlight") :
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
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getActualLoad(Interval(day).firstDay(), t);
		QString bgCol = 
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(sameTimeNextDay(day) - 1) ?
			  "completedlight" : "bookedlight")) :
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
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getPlanLoad(Interval(day).firstDay(), r);
		QString bgCol = 
			t->isPlanActive(Interval(day).firstDay()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isPlanBuffer(Interval(day).firstDay()) ?
			  "booked" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) ? "vacation" :
			(r == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids = r->getPlanProjectIDs(Interval(day).firstDay(), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::dailyTaskActual(Task* t, Resource* r)
{
	if (!hidePlan)
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
			 (r == 0 && !t->isActualBuffer(Interval(day).firstDay())
			  ? "booked" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) ? "vacation" :
			(r == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids = r->getActualProjectIDs(Interval(day).firstDay(), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
			reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::weeklyResourcePlan(Resource* r, Task* t)
{
	if (hidePlan)
		return;

	if (showActual)
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
		if (showPIDs)
		{
			QString pids = r->getPlanProjectIDs(Interval(week).firstWeek(), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
			reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::weeklyResourceActual(Resource* r, Task* t)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = r->getActualLoad(Interval(week).firstWeek(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(sameTimeNextWeek(week) - 1) ?
			  "completedlight" : "bookedlight")) :
			isSameWeek(project->getNow(), week) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids = r->getActualProjectIDs(Interval(week).firstWeek(),
												  t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
			reportLoad(load, bgCol, !r->isGroup());
	}
}

void 
ReportHtml::weeklyTaskPlan(Task* t, Resource* r)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getPlanLoad(Interval(week).firstWeek(), r);
		QString bgCol = 
			t->isPlanActive(Interval(week).firstWeek()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isPlanBuffer(Interval(week).firstWeek())
			  ? "booked" : "bookedlight")) :
			isSameWeek(project->getNow(), week) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::weeklyTaskActual(Task* t, Resource* r)
{
	if (!hidePlan)
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
			 (r == 0 && !t->isActualBuffer(Interval(week).firstWeek())
			  ? "booked" : "bookedlight")) :
			isSameWeek(project->getNow(), week) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::monthlyResourcePlan(Resource* r, Task* t)
{
	if (hidePlan)
		return;

	if (showActual)
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
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getActualLoad(Interval(month).firstMonth(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(sameTimeNextMonth(month) - 1) ?
			  "completedlight" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::monthlyTaskPlan(Task* t, Resource* r)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = t->getPlanLoad(Interval(month).firstMonth(), r);
		QString bgCol = 
			t->isPlanActive(Interval(month).firstMonth()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isPlanBuffer(Interval(month).firstMonth())
			  ? "booked" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::monthlyTaskActual(Task* t, Resource* r)
{
	if (!hidePlan)
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
			 (r == 0 && !t->isActualBuffer(Interval(month).firstMonth())
			  ? "booked" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::taskName(Task* t, Resource* r, bool big)
{
	QString spaces = "";
	int fontSize = big ? 150 : 100;
	if (resourceSortCriteria == CoreAttributesList::TreeMode)
		for (Resource* rp = r ; rp != 0; fontSize = (int) (fontSize * 0.85))
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			rp = rp->getParent();
		}

	mt.clear();
	mt.addMacro(new Macro(KW("taskid"), t->getId(), defFileName,
						  defFileLine));

	if (taskSortCriteria == CoreAttributesList::TreeMode)
	{
		Task* tp = t->getParent();
		for ( ; tp != 0; fontSize = (int) (fontSize * 0.85))
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			tp = tp->getParent();
		}
		s << "<td class=\""
		  << (r == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces
		  << "<span style=\"font-size:" 
		  << QString().sprintf("%d", fontSize) << "%\">";
		if (r == 0)
			s << "<a name=\"task_" << t->getId() << "\"></a>";
		s << generateUrl(KW("taskname"), t->getName());
		s << "</span></td>" << endl;
	}
	else
	{
		s << "<td class=\""
		  << (r == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces;
		if (r == 0)
			s << "<a name=\"task_" << t->getFullId() << "\"></a>";
		s << generateUrl(KW("taskname"), t->getName());
		s << "</td>" << endl;
	}
}

void
ReportHtml::resourceName(Resource* r, Task* t, bool big)
{
	QString spaces = "";
	int fontSize = big ? 150 : 100;
	if (taskSortCriteria == CoreAttributesList::TreeMode)
		for (Task* tp = t; tp != 0; fontSize = (int) (fontSize * 0.85))
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			tp = tp->getParent();
		}

	mt.clear();
	mt.addMacro(new Macro(KW("resourceid"), r->getId(), defFileName,
						  defFileLine));
	
	if (resourceSortCriteria == CoreAttributesList::TreeMode)
	{
		Resource* rp = r->getParent();
		for ( ; rp != 0; fontSize = (int) (fontSize * 0.85))
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			rp = rp->getParent();
		}
		s << "<td class=\""
		  << (t == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces
		  << "<span style=\"font-size:" 
		  << QString().sprintf("%d", fontSize) << "%\">";
		if (t == 0)
			s << "<a name=\"resource_" << r->getId() << "\"></a>";
		s << generateUrl(KW("resourcename"), r->getName());
		s << "</span></td>" << endl;
	}
	else
	{
		s << "<td class=\""
		  << (t == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces;
		if (t == 0)
			s << "<a name=\"resource_" << r->getId() << "\"></a>";
		s << generateUrl(KW("resourcename"), r->getName());
		s << "</td>" << endl;
	}
}

void
ReportHtml::planResources(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" style=\"text-align:left\">"
	  << "<span style=\"font-size:0.8em\">";
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
	s << "</span></td>" << endl;
}

void
ReportHtml::actualResources(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" style=\"text-align:left\">"
	  << "<span style=\"font-size:0.8em\">";
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
	s << "</span></td>" << endl;
}

void
ReportHtml::generateDepends(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\"><span style=\"font-size:0.8em\">";
	bool first = TRUE;
	for (Task* d = t->firstPrevious(); d != 0;
		 d = t->nextPrevious())
	{
		if (!first)
			s << ", ";
		else
			first = FALSE;
		s << QString().sprintf("%d", d->getIndex());
	}
	s << "</span</td>" << endl;
}

void
ReportHtml::generateFollows(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<span style=\"font-size:0.8em\">";
	bool first = TRUE;
	for (Task* d = t->firstFollower(); d != 0;
		 d = t->nextFollower())
	{
		if (!first)
			s << ", ";
		s << QString().sprintf("%d", d->getIndex());
		first = FALSE;
	}
	s << "</span</td>" << endl;
}

void
ReportHtml::generateResponsibilities(Resource*r, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<span style=\"font-size:0.8em\">";
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
	s << "</span></td>" << endl;
}

void
ReportHtml::planSchedule(Resource* r, Task* t)
{
	s << "<td class=\""
	  << (t == 0 ? "default" : "defaultlight") 
	  << "\" style=\"text-align:left\">";

	BookingList planJobs = r->getPlanJobs();
	planJobs.setAutoDelete(TRUE);
	time_t prevTime = 0;
	for (Booking* b = planJobs.first(); b != 0; b = planJobs.next())
	{
		if ((t == 0 || t == b->getTask()) && 
			Interval(start, end).overlaps(Interval(b->getStart(),
												   b->getEnd())))
		{
			if (!isSameDay(prevTime, b->getStart()))
			{
				s << "<p><span style=\"font-size:160%\">"
					<< time2weekday(b->getStart()) << ", "
					<< time2date(b->getStart()) << "</span><p>" << endl;
			}
			s << "&nbsp;&nbsp;&nbsp;"
				<< time2time(b->getStart()) << " - "
				<< time2time(b->getEnd());
			if (t == 0)
				s << " " << htmlFilter(b->getTask()->getName());
			s << "<br>" << endl;
			prevTime = b->getStart();
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
	actualJobs.setAutoDelete(TRUE);
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
		s << scaledLoad(load);
		if (bold)
			s << "</b>";
		s << "</td>" << endl;
	}
	else
		s << "<td class=\""
		  << bgCol << "\">&nbsp;</td>" << endl;
}

void
ReportHtml::reportPIDs(const QString& pids, const QString bgCol, bool bold)
{
	s << "<td class=\""
	  << bgCol << "\" style=\"white-space:nowrap\">";
	if (bold)
		s << "<b>";
	s << pids;
	if (bold)
		s << "</b>";
	s << "</td>" << endl;
}

bool
ReportHtml::setUrl(const QString& key, const QString& url)
{
	if (urls.find(key) == urls.end())
		return FALSE;

	urls[key] = url;
	return TRUE;
}

const QString*
ReportHtml::getUrl(const QString& key) const
{
	if (urls.find(key) == urls.end() || urls[key] == "")
		return 0;
	return &urls[key];
}

QString
ReportHtml::generateUrl(const QString& key, const QString& txt)
{
	if (getUrl(key))
	{
		mt.setLocation(defFileName, defFileLine);
		return QString("<a href=\"") + mt.expand(*getUrl(key))
			+ "\">" + htmlFilter(txt) + "</a>";
	}
	else
		return htmlFilter(txt);
}

