/*
 * HTMLResourceReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <qfile.h>
#include "Project.h"
#include "ResourceList.h"
#include "HTMLResourceReport.h"

bool
HTMLResourceReport::generate()
{
	QFile f(fileName);
	if (!f.open(IO_WriteOnly))
	{
		qWarning("Cannot open report file %s!\n",
				 fileName.latin1());
		return FALSE;
	}
	s.setDevice(&f);
	reportHTMLHeader();
	s << "<table border=\"0\" cellpadding=\"1\">" << endl;

	if (!generateTableHeader())
		return FALSE;

	for (Resource* r = project->resourceListFirst(); r != 0;
		 r = project->resourceListNext())
	{
		if (isResourceHidden(r))
			continue;

		// Resource line
		s << "<tr>";
		for (QStringList::Iterator it = columns.begin();
			 it != columns.end(); ++it )
		{
			if (*it == "resource")
			{
				s << "<td class=\"available\" nowrap "
				  << "style=\"text-align:left\"><b>"
				  << htmlFilter(r->getName()) << "</b></td>";
			}
			else if (*it == "start")
				s << "<td class=\"available\" nowrap>&nbsp;</td>";
			else if (*it == "end")
				s << "<td class=\"available\" nowrap>&nbsp;</td>";
			else if (*it == "workdays")
				s << "<td class=\"available\" "
				  << "style=\"text-align:right\"><b>"
				  << QString().sprintf(
					  "%1.1f", r->getLoad(Interval(start, end)))
				  << "</b></td>";
			else if (*it == "schedule")
				s << "<td class=\"available\" nowrap>&nbsp;</td>";
			else if (*it == "daily")
				dailyResourcePlan(r);
			else if (*it == "weekly")
				weeklyResourcePlan(r);
			else if (*it == "monthly")
				monthlyResourcePlan(r);
		}
		s << "</tr>\n";

		TaskList filteredTaskList;
		filterTaskList(filteredTaskList);
		TaskList ftl;
		ftl.setAutoDelete(FALSE);
		// Remove tasks that the current resource is not assigned to.
		for (Task* t = filteredTaskList.first(); t != 0;
			 t = filteredTaskList.next())
		{
			if (r->getLoad(Interval(start, end), t) > 0.0)
				ftl.append(t);
		}
		filteredTaskList = ftl;
		sortTaskList(filteredTaskList);

		// Task lines for each resource
		for (Task* t = filteredTaskList.first(); t != 0;
			 t = filteredTaskList.next())
		{
			s << "<tr>";
			for (QStringList::Iterator it = columns.begin();
				 it != columns.end(); ++it )
			{
				if (*it == "resource")
				{
					s << "<td class=\"default\" nowrap "
					  << "style=\"text-align:left\">";
					if (taskSortCriteria == TaskList::TaskTree)
						for (Task* tp = t; tp != 0; tp = tp->getParent())
							s << "&nbsp;&nbsp;&nbsp;&nbsp;";
					else
						s << "&nbsp;&nbsp;&nbsp;&nbsp;";
					s << htmlFilter(t->getName())
					  << "</font></td>";
				}
				else if (*it == "start")
					s << "<td class=\"default\" nowrap>"
					  << time2ISO(t->getStart())
					  << "</td>" << endl;
				else if (*it == "end")
					s << "<td class=\"default\" nowrap>"
					  << time2ISO(t->getEnd())
					  << "</td>" << endl;
				else if (*it == "workdays")
					s << "<td class=\"default\" "
					  << "style=\"text-align:right\">"
					  << QString().sprintf(
						  "%1.1f", r->getLoad(Interval(start, end), t))
					  << "</td>";
				else if (*it == "schedule")
					schedulePlan(r, t);
				else if (*it == "daily")
					dailyTaskPlan(r, t);
				else if (*it == "weekly")
					weeklyTaskPlan(r, t);
				else if (*it == "monthly")
					monthlyTaskPlan(r, t);
			}
			s << "</tr>\n";
		}
	}

	s << "</table>";
	reportHTMLFooter();

	f.close();
	return TRUE;
}

bool
HTMLResourceReport::generateTableHeader()
{
	// Header line 1
	s << "<tr>";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "resource")
			s << "<td class=\"headerbig\" rowspan=\"2\">Resource/Task</td>";
		else if (*it == "start")
			s << "<td class=\"headerbig\" rowspan=\"2\">Start</td>";
		else if (*it == "end")
			s << "<td class=\"headerbig\" rowspan=\"2\">End</td>";
		else if (*it == "workdays")
			s << "<td class=\"headerbig\" rowspan=\"2\">Work Days</td>";
		else if (*it == "schedule")
			s << "<td class=\"headerbig\" rowspan=\"2\">Schedule</td>";
		else if (*it == "daily")
			htmlDayHeaderMonths();
		else if (*it == "weekly")
			htmlWeekHeaderMonths();
		else if (*it == "monthly")
			htmlMonthHeaderYears();
		else
		{
			qWarning("Unknown column ID %s", (*it).latin1());
			return FALSE;
		}
	}
	s << "</tr>" << endl;

	// Header line 2
	s << "<tr>";
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
	s << "</tr>" << endl;

	return TRUE;
}

void
HTMLResourceReport::schedulePlan(Resource* r, Task* t)
{
	s << "<td class=\"default\" style=\"text-align:left\">";

	bool first = TRUE;
	for (Booking* b = r->jobsFirst(); b != 0; b = r->jobsNext())
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
HTMLResourceReport::dailyResourcePlan(Resource* r)
{
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getLoadOnDay(day);

		s << "<td class=\""
		  << (load > r->getMinEffort() * r->getEfficiency() ? "booked" :
			  isSameDay(project->getNow(), day) ? "today" :
			  isWeekend(day) ? "weekend" :
			  project->isVacation(day) || r->hasVacationDay(day) ?
			  "vacation" : "available")
		  << "\">";
		if (load > 0.0)
			s << QString().sprintf("<b>%3.1f</b>", load);
		s << "</td>";
	}
}

void 
HTMLResourceReport::dailyTaskPlan(Resource* r, Task* t)
{
	for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
	{
		double load = r->getLoadOnDay(day, t);

		s << "<td class=\""
		  << (load > 0.0 ? "booked" :
			  isSameDay(project->getNow(), day) ? "today" :
			  isWeekend(day) ? "weekend" :
			  project->isVacation(day) ? "vacation" : "default")
		  << "\">";
		if (load > 0.0)
			s << QString().sprintf("%3.1f", load);
		s << "</td>";
	}
}

void
HTMLResourceReport::weeklyResourcePlan(Resource* r)
{
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = r->getLoad(Interval(week, sameTimeNextWeek(week)));

		s << "<td class=\""
		  << (load > r->getMinEffort() * r->getEfficiency() ? "booked" :
			  isSameWeek(project->getNow(), week) ? "today" :
			  "available")
		  << "\">";
		if (load > 0.0)
			s << QString().sprintf("<b>%3.1f</b>", load);
		s << "</td>";
	}
}

void 
HTMLResourceReport::weeklyTaskPlan(Resource* r, Task* t)
{
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = r->getLoad(Interval(week, sameTimeNextWeek(week)), t);

		s << "<td class=\""
		  << (load > 0.0 ? "booked" :
			  isSameWeek(project->getNow(), week) ? "today" :
			  "default")
		  << "\">";
		if (load > 0.0)
			s << QString().sprintf("%3.1f", load);
		s << "</td>";
	}
}

void
HTMLResourceReport::monthlyResourcePlan(Resource* r)
{
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getLoad(Interval(month, sameTimeNextMonth(month)));

		s << "<td class=\""
		  << (load > r->getMinEffort() * r->getEfficiency() ? "booked" :
			  isSameMonth(project->getNow(), month) ? "today" :
			  "available")
		  << "\">";
		if (load > 0.0)
			s << QString().sprintf("<b>%1.1f</b>", load);
		s << "</td>";
	}
}

void 
HTMLResourceReport::monthlyTaskPlan(Resource* r, Task* t)
{
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getLoad(Interval(month, sameTimeNextMonth(month)), t);

		s << "<td class=\""
		  << (load > 0.0 ? "booked" :
			  isSameMonth(project->getNow(), month) ? "today" :
			  "default")
		  << "\">";
		if (load > 0.0)
			s << QString().sprintf("%1.1f", load);
		s << "</td>";
	}
}
