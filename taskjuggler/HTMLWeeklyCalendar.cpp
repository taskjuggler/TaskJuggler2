/*
 * HTMLWeeklyCalendar.cpp - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdlib.h>

#include <qfile.h>

#include "Project.h"
#include "HTMLWeeklyCalendar.h"
#include "ExpressionTree.h"
#include "Utility.h"

HTMLWeeklyCalendar::HTMLWeeklyCalendar(Project* p, const QString& f, time_t s,
									   time_t e, const QString& df, int dl) :
	ReportHtml(p, f, s, e, df, dl)
{
	columns.append("name");

	// show all tasks
	hideTask = new ExpressionTree(new Operation(0));
	// hide all resources
	hideResource = new ExpressionTree(new Operation(1));

	taskSortCriteria[0] = CoreAttributesList::TreeMode;
	taskSortCriteria[1] = CoreAttributesList::PlanStartUp;
	taskSortCriteria[2] = CoreAttributesList::PlanEndUp;
}

bool
HTMLWeeklyCalendar::generate()
{
	if (!open())
		return FALSE;
	reportHTMLHeader();

	TaskList filteredList;
	filterTaskList(filteredList, 0);
	sortTaskList(filteredList);

	generateCalendar(filteredList);

/*	for (Task* t = filteredList.first(); t != 0; t = filteredList.next())
	{
		generatePlanTask(t, 0);
		if (showActual)
			generateActualTask(t, 0);

		ResourceList filteredResourceList;
		filterResourceList(filteredResourceList, t);
		sortResourceList(filteredResourceList);

		for (Resource* r = filteredResourceList.first(); r != 0;
			 r = filteredResourceList.next())
		{
			generatePlanResource(r, t);
			if (showActual)
				generateActualResource(r, t);
		}
	}*/
	reportHTMLFooter();

	f.close();
	return TRUE;
}

bool
HTMLWeeklyCalendar::generateCalendar(TaskList& filteredList)
{
	for (time_t week = beginOfWeek(start);
		 week <= sameTimeNextWeek(beginOfWeek(end)); )
	{
		time_t wd = week;
		s << "<tr>";
		for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
		{
			s << "<td width=\"14.2%\" class=\"";
			s << (isSameDay(project->getNow(), wd) ? "today" :
				  isWeekend(wd) ? "weekend" : "headersmall");
			s << "\">"
				<< "<table width=\"100%\" style=\"text-align=\"left\">"
				<< "<tr>"
				<<   "<td width=\"25%\" rowspan=\"2\" "
				     "style=\"font-size:250%; text-align:center\">" 
				<<   QString().sprintf("%d", dayOfMonth(wd)) << "</td>"
				<<   "<td width=\"105%\">" 
				<<   htmlFilter(dayOfWeekName(wd)) << "</td>"
				<< "</tr>"
				<< "<tr>"
				<<   "<td style=\"font-size:110%\">" 
				<<   monthAndYear(wd) << "</td>"
				<< "</tr>"
				<< "</table></td>" << endl;
		}
		s << "</tr><tr>" << endl;
		wd = week;
		for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
		{
			s << "<td width=\"14.2%\" class=\"default\" "
				 "style=\"vertical-align:top\">";
			bool first = TRUE;
			for (Task* t = filteredList.first(); t; t = filteredList.next())
				if (t->isPlanActive(Interval(wd, sameTimeNextDay(wd))))
				{
					if (first)
					{
						s << "<table width=\"100%\" height=\"100%\" "
							 "style=\"font-size:100%\">" << endl;
						first = FALSE;
					}
					generatePlanTask(t, 0);
				}
			if (!first)
				s << "</table>" << endl;
			else
				s << "<p>&nbsp;</p>" << endl;	
			s << "</td>";
		}
		s << "</tr>";
		week = wd;	
	}

	return TRUE;
}

