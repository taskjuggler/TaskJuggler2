/*
 * HTMLStatusReport.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLStatusReport_h_
#define _HTMLStatusReport_h_

#include "ReportHtml.h"

class Project;
class TaskList;

class HTMLStatusReport : public ReportHtml
{
public:
	HTMLStatusReport(Project* p, const QString& f, time_t s, time_t e,
					   const QString& df, int dl);
	virtual ~HTMLStatusReport() { }

	bool generate();

private:
	HTMLStatusReport() { }	// don't call this directly

	bool generateTable(TaskList& filteredTaskList,
					   ResourceList& filteredResourceList);
} ;

#endif

