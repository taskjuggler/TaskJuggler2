/*
 * HTMLWeeklyCalendar.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLWeeklyCalendar_h_
#define _HTMLWeeklyCalendar_h_

#include "ReportHtml.h"

class Project;
class TaskList;

class HTMLWeeklyCalendar : public ReportHtml
{
public:
	HTMLWeeklyCalendar(Project* p, const QString& f, time_t s, time_t e,
					   const QString& df, int dl);
	virtual ~HTMLWeeklyCalendar() { }

	bool generate();

private:
	HTMLWeeklyCalendar() { }	// don't call this directly

	bool generateCalendar(TaskList& filteredTaskList,
						  ResourceList& filteredResourceList);
} ;

#endif

