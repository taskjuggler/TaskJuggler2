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

	resourceSortCriteria[0] = CoreAttributesList::TreeMode;
	resourceSortCriteria[1] = CoreAttributesList::NameUp;
	resourceSortCriteria[2] = CoreAttributesList::IdUp;
}

bool
HTMLWeeklyCalendar::generate()
{
	if (!open())
		return FALSE;
	reportHTMLHeader();

	TaskList filteredTaskList;
	filterTaskList(filteredTaskList, 0);
	sortTaskList(filteredTaskList);

	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList, 0);
	sortResourceList(filteredResourceList);
	
	generateCalendar(filteredTaskList, filteredResourceList);

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
HTMLWeeklyCalendar::generateCalendar(TaskList& filteredTaskList, ResourceList&
									 filteredResourceList)
{
	// TODO: Make first day of week configurable.
	for (time_t week = beginOfWeek(start, TRUE);
		 week <= sameTimeNextWeek(beginOfWeek(end, TRUE)); )
	{
		time_t wd = week;
		/* Generate table row that contains the day of the month, the month
		 * and the year. The row starts with a cell that shows the week number
		 * of the first day of the week. */
		s << "<tr>";
		s << "<td width=\"5.5%\" class=\"headersmall\" "
			"style=\"font-size:110%\">"
			<< "Week " << QString().sprintf("%d",
										   	weekOfYear(wd, weekStartsMonday))
		   	<< "</td>" << endl;
		for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
		{
			s << "<td width=\"13.5%\" class=\"";
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
		s << "</tr>" << endl;

		if (!filteredTaskList.isEmpty())
		{
			// Generate a row with lists the tasks for each day.
			s << "<tr><td width=\"5.5%\" class=\"default\">&nbsp;</td>" << endl;
			wd = week;
			for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
			{
				/* Misuse the class member start and end to limit the scope of
			     * the information listed. */
				time_t savedStart = start;
				time_t savedEnd = end;
				start = wd;
				end = sameTimeNextDay(wd);
				s << "<td width=\"13.5%\" class=\"default\" "
					"style=\"vertical-align:top\">";
				bool first = TRUE;
				for (Task* t = filteredTaskList.first(); t;
					 t = filteredTaskList.next())
				{
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
				}
				if (!first)
					s << "</table>" << endl;
				else
					s << "<p>&nbsp;</p>" << endl;	
				s << "</td>";
				start = savedStart;
				end = savedEnd;
			}
			s << "</tr>";
		}

		if (!filteredResourceList.isEmpty())
		{
			// Generate a table row which lists the resources for each day.	
			s << "<tr><td width=\"5.5%\" class=\"default\">&nbsp;</td>" << endl;
			wd = week;
			for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
			{
				/* Misuse the class member start and end to limit the scope of
			     * the information listed. */
				time_t savedStart = start;
				time_t savedEnd = end;
				start = wd;
				end = sameTimeNextDay(wd);
				s << "<td width=\"13.5%\" class=\"default\" "
					"style=\"vertical-align:top\">";
				bool first = TRUE;
				for (Resource* r = filteredResourceList.first(); r;
					 r = filteredResourceList.next())
				{
					if (r->getPlanLoad(Interval(wd, sameTimeNextDay(wd))) > 0.0)
					{
						if (first)
						{
							s << "<table width=\"100%\" height=\"100%\" "
								"style=\"font-size:100%\">" << endl;
							first = FALSE;
						}
						generatePlanResource(r, 0);
					}
				}
				if (!first)
					s << "</table>" << endl;
				else
					s << "<p>&nbsp;</p>" << endl;	
				s << "</td>";
				start = savedStart;
				end = savedEnd;
			}
			s << "</tr>";
		}

		week = wd;	
	}

	return TRUE;
}

