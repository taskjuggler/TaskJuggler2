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

void
HTMLResourceReport::dailyResourcePlan(Resource* r)
{
	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getLoadOnDay(day);

		s.reset();
		s.setf(QTextStream::hex);
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

void HTMLResourceReport::dailyTaskPlan(Resource* r, Task* t)
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
		s.precision(3);
		if (load > 0.0)
			s << QString().sprintf("%3.1f", load);
		s << "</td>";
	}
}

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

	// Header line 1
	s << "<tr>";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == "resource")
			s << "<td class=\"headerbig\" rowspan=\"2\">Resource/Task</td>";
		else if (*it == "daily")
			htmlMonthHeader(); 
		else
		{
			qWarning("Unknown column ID");
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
			htmlDayHeader();
	}
	s << "</tr>" << endl;

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
			else if (*it == "daily")
				dailyResourcePlan(r);
		}
		s << "</tr>\n";

		// Task lines for each resource
		for (Task* t = project->taskListFirst(); t != 0;
			 t = project->taskListNext())
			if (r->isAssignedTo(t) && !isTaskHidden(t) &&
				r->getLoad(Interval(start, end), t) > 0.0)
			{
				s << "<tr>";
				for (QStringList::Iterator it = columns.begin();
					 it != columns.end(); ++it )
				{
					if (*it == "resource")
					{
						s << "<td class=\"default\" nowrap "
						  << "style=\"text-align:left\">"
						  << "&nbsp;&nbsp;&nbsp;&nbsp;"
						  << htmlFilter(t->getName())
						  << "</font></td>";
					}
					else if (*it == "daily")
						dailyTaskPlan(r, t);
				}
				s << "</tr>\n";
			}
	}

	s << "</table>";
	reportHTMLFooter();

	f.close();
	return TRUE;
}

