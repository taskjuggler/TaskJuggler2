/*
 * HTMLTaskReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdlib.h>

#include <qfile.h>

#include "Project.h"
#include "HTMLTaskReport.h"
#include "ExpressionTree.h"

HTMLTaskReport::~HTMLTaskReport()
{
	delete rollUpTask;
}

bool
HTMLTaskReport::generate()
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
	s << "<table border=\"0\" cellpadding=\"1\">\n" << endl;

	if (!generateTableHeader())
		return FALSE;

	TaskList filteredList;
	filterTaskList(filteredList);
	sortTaskList(filteredList);

	// Create a task->index no. dictionary.
	int i = 1;
	QDict<int> idxDict;
	idxDict.setAutoDelete(TRUE);
	for (Task* t = filteredList.first(); t != 0;
		 t = filteredList.next(), ++i)
		idxDict.insert(t->getId(), new int(i));

	i = 1;
	for (Task* t = filteredList.first(); t != 0;
		 t = filteredList.next(), ++i)
	{
		s << "<tr valign=\"center\">";
		for (QStringList::Iterator it = columns.begin(); it != columns.end();
			 ++it )
		{
			if (*it == "no")
				s << "<td class=\"default\" rowspan=\""
				  << (showActual ? "2" : "1") << "\">"
				  << QString().sprintf("%d.", i) << "</td>";
			else if (*it == "taskId")
				s << "<td class=\"default\" rowspan=\""
				  << (showActual ? "2" : "1") << "\" nowrap>"
				  << htmlFilter(t->getId()) << "</td>";
			else if (*it == "name")
				generateTaskName(t);
			else if (*it == "start")
				s << "<td class=\""
				  << (t->isStartOk() ? "default" : "milestone")
				  << "\" nowrap>"
				  << time2ISO(t->getStart())
				  << "</td>" << endl;
			else if (*it == "end")
				s << "<td class=\""
				  << (t->isEndOk() ? "default" : "milestone")
				  << "\" nowrap>"
				  << time2ISO(t->getEnd())
				  << "</td>" << endl;
			else if (*it == "minStart")
				s << "<td class=\"default\" rowspan=\""
				  << (showActual ? "2" : "1") << "\">"
				  << time2ISO(t->getMinStart())
				  << "</td>" << endl;
			else if (*it == "maxStart")
				s << "<td class=\"default\" rowspan=\""
				  << (showActual ? "2" : "1") << "\">"
				  << time2ISO(t->getMaxStart())
				  << "</td>" << endl;
			else if (*it == "resources")
				generateResources(t);
			else if (*it == "depends")
				generateDepends(t, idxDict);
			else if (*it == "follows")
				generateFollows(t, idxDict);
			else if (*it == "note")
			{
				s << "<td class=\"default\" rowspan=\""
				  << (showActual ? "2" : "1")
				  << "\" style=\"text-align:left\">"
				  << "<font size=\"-2\">";
				s << htmlFilter(t->getNote());
				s << "</font></td>" << endl;
			}
			else if (*it == "costs")
			{
				s << "<td class=\"default\" rowspan=\""
				  << (showActual ? "2" : "1")
				  << " style=\"text-align:right\">"
				  << QString().sprintf("%5.3f", t->getPlanCosts())
				  << "</td>" << endl;
			}
			else if (*it == "priority")
			{
				s << "<td class=\"default\""
				  << " style=\"text-align:right\">"
				  << QString().sprintf("%d", t->getPriority())
				  << "</td>" << endl;
			}
			else if (*it == "daily")
				generateDailyPlan(t);
			else if (*it == "weekly")
				generateWeeklyPlan(t);
			else if (*it == "monthly")
				generateMonthlyPlan(t);
		}
		s << "</tr>" << endl;

		if (showActual)
		{
			s << "<tr>" << endl;
			for (QStringList::Iterator it = columns.begin();
				 it != columns.end();
				 ++it )
			{
				if (*it == "start")
				{
					s << "<td class=\""
					  << (t->isActualStartOk() ? "default" : "milestone")
					  << "\" nowrap>"
					  << time2ISO(t->getActualStart())
					  << "</td>" << endl;
				}
				else if (*it == "end")
				{
					s << "<td class=\""
					  << (t->isActualEndOk() ? "default" : "milestone")
					  << "\" nowrap>"
					  << time2ISO(t->getActualEnd())
					  << "</td>" << endl;
				}
				else if (*it == "resources")
				{
					s << "<td>&nbsp;</td>" << endl;
				}
				else if (*it == "costs")
				{
					s << "<td>&nbsp;</td>" << endl;
				}
				if (*it == "daily")
					generateDailyActual(t);
				else if (*it == "weekly")
					generateWeeklyActual(t);
				else if (*it == "monthly")
					generateMonthlyActual(t);
			}
			s << "</tr>" << endl;
		}
	}
	s << "</table>" << endl;
	reportHTMLFooter();

	f.close();
	return TRUE;
}

bool
HTMLTaskReport::generateTableHeader()
{
	// Header line 1
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "no")
			s << "<td class=\"headerbig\" rowspan=\"2\">No.</td>";
		else if (*it == "taskId")
			s << "<td class=\"headerbig\" rowspan=\"2\">Task ID</td>";
		else if (*it == "name")
			s << "<td class=\"headerbig\" rowspan=\"2\">Task/Milestone</td>";
		else if (*it == "start")
			s << "<td class=\"headerbig\" rowspan=\"2\">Start</td>";
		else if (*it == "end")
			s << "<td class=\"headerbig\" rowspan=\"2\">End</td>";
		else if (*it == "minStart")
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. Start</td>";
		else if (*it == "maxStart")
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. Start</td>";
		else if (*it == "resources")
			s << "<td class=\"headerbig\" rowspan=\"2\">Resources</td>";
		else if (*it == "depends")
			s << "<td class=\"headerbig\" rowspan=\"2\">Dependencies</td>";
		else if (*it == "follows")
			s << "<td class=\"headerbig\" rowspan=\"2\">Followers</td>";
		else if (*it == "note")
			s << "<td class=\"headerbig\" rowspan=\"2\">Note</td>";
		else if (*it == "costs")
			s << "<td class=\"headerbig\" rowspan=\"2\">Costs</td>";
		else if (*it == "priority")
			s << "<td class=\"headerbig\" rowspan=\"2\">Priority</td>";
		else if (*it == "daily")
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";
			htmlDayHeaderMonths();
		}
		else if (*it == "weekly")
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";
			htmlWeekHeaderMonths();
		}
		else if (*it == "monthly")
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";
			htmlMonthHeaderYears();
		}
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
HTMLTaskReport::generateTaskName(Task* t)
{
	if (taskSortCriteria == TaskList::TaskTree)
	{
		int indent;
		Task* tp = t->getParent();
		QString spaces = "";
		for (indent = 0; tp != 0; ++indent)
		{
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
			tp = tp->getParent();
		}
		s << "<td class=\"task\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" nowrap>"
		  << spaces
		  << "<font size=\""
		  << (2 - indent < 0 ? '-' : '+') 
		  << (2 - indent < 0 ? -(2 - indent) : 2 - indent) << "\">"
		  << htmlFilter(t->getName())
		  << "</font></td>" << endl;
	}
	else
		s << "<td class=\"task\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" nowrap>"
		  << htmlFilter(t->getName())
		  << "</td>" << endl;
}

void
HTMLTaskReport::generateResources(Task* t)
{
	s << "<td class=\"default\" style=\"text-align:left\">"
	  << "<font size=\"-2\">";
	bool first = TRUE;
	for (Resource* r = t->firstBookedResource(); r != 0;
		 r = t->nextBookedResource())
	{
		if (!first)
			s << ", ";
					
		s << htmlFilter(r->getName());
		first = FALSE;
	}
	s << "</font></td>" << endl;
}

void
HTMLTaskReport::generateDepends(Task* t, const QDict<int>& idxDict)
{
	s << "<td class=\"default\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\"><font size=\"-2\">";
	bool first = TRUE;
	for (Task* d = t->firstPrevious(); d != 0;
		 d = t->nextPrevious())
	{
		if (idxDict[d->getId()])
		{
			if (!first)
				s << ", ";
			s << QString().sprintf("%d", *(idxDict[d->getId()]));
			first = FALSE;
		}
	}
	s << "</font></td>" << endl;
}

void
HTMLTaskReport::generateFollows(Task* t, const QDict<int>& idxDict)
{
	s << "<td class=\"default\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<font size=\"-2\">";
	bool first = TRUE;
	for (Task* d = t->firstFollower(); d != 0;
		 d = t->nextFollower())
	{
		if (idxDict[d->getId()])
		{
			if (!first)
				s << ", ";
			s << QString().sprintf("%d", *(idxDict[d->getId()]));
			first = FALSE;
		}
	}
	s << "</font></td>" << endl;
}

void
HTMLTaskReport::generateDailyPlan(Task* t)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getLoadOnDay(day);
		QString bgCol = 
			(t->isMilestone() && t->isActiveToday(day) ?
			 "milestone" :
			 (t->isActiveToday(day) &&
			  !(isWeekend(day) && load == 0.0)) ? "booked" :
			 isSameDay(project->getNow(), day) ? "today" :
			 isWeekend(day) ? "weekend" :
			 project->isVacation(day) ? "vacation" :
			 "default");
		if (load > 0.0)
			s << "<td class=\""
			  << bgCol << "\">"
			  << QString().sprintf("%3.1f", load)
			  << "</td>" << endl;
		else
			s << "<td class=\""
			  << bgCol << "\">&nbsp;</td>" << endl;
	}
}

void
HTMLTaskReport::generateDailyActual(Task* t)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getLoadOnDay(day);
		QString bgCol = 
			(isSameDay(project->getNow(), day) ? "today" :
			 (t->isDayCompleted(day) &&
			  (load > 0.0 || t->isMilestone())) ? "completed" :
			 isWeekend(day) ? "weekend" :
			 project->isVacation(day) ? "vacation" :
			 "default");
		s << "<td class=\""
		  << bgCol << "\">&nbsp;</td>";
	}
}

void
HTMLTaskReport::generateWeeklyPlan(Task* t)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getLoad(Interval(week,
										  sameTimeNextWeek(week)));
		QString bgCol = 
			(t->isMilestone() && t->isActiveThisWeek(week) ?
			 "milestone" :
			 t->isActiveThisWeek(week) ? "booked" :
			 isSameWeek(project->getNow(), week) ? "today" :
			 "default");
		if (load > 0.0)
			s << "<td class=\""
			  << bgCol << "\">"
			  << QString().sprintf("%3.1f", load)
			  << "</td>" << endl;
		else
			s << "<td class=\""
			  << bgCol << "\">&nbsp;</td>" << endl;
	}
}

void
HTMLTaskReport::generateWeeklyActual(Task* t)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t week = beginOfWeek(start); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getLoad(
			Interval(week, sameTimeNextWeek(week) - 1));
		QString bgCol = 
			(isSameWeek(project->getNow(), week) ? "today" :
			 (t->isDayCompleted(week) &&
			  (load > 0.0 || t->isMilestone())) ? "completed" :
			 "default");
		s << "<td class=\""
		  << bgCol << "\">&nbsp;</td>";
	}
}

void
HTMLTaskReport::generateMonthlyPlan(Task* t)
{
	s << "<td class=\"headersmall\">Plan</td>" << endl;
	for (time_t day = beginOfMonth(start); day < end;
		 day = sameTimeNextMonth(day))
	{
		double load = t->getLoadOnMonth(day);
		QString bgCol = 
			(t->isMilestone() && t->isActiveThisMonth(day) ?
			 "milestone" :
			 (t->isActiveThisMonth(day)) ? "booked" :
			 isSameMonth(project->getNow(), day) ? "today" :
			 "default");
		if (load > 0.0)
			s << "<td class=\""
			  << bgCol << "\">"
			  << QString().sprintf("%4.1f", load)
			  << "</td>" << endl;
		else
			s << "<td class=\""
			  << bgCol << "\">&nbsp;</td>" << endl;
	}
}

void
HTMLTaskReport::generateMonthlyActual(Task* t)
{
	s << "<td class=\"headersmall\">Actual</td>" << endl;
	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = t->getLoad(
			Interval(month, sameTimeNextMonth(month) - 1));
		QString bgCol = 
			(isSameMonth(project->getNow(), month) ? "today" :
			 (t->isDayCompleted(month) &&
			  (load > 0.0 || t->isMilestone())) ? "completed" :
			 "default");
		s << "<td class=\""
		  << bgCol << "\">&nbsp;</td>";
	}
}
