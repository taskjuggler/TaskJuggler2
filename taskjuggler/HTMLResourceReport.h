/*
 * HTMLResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLResourceReport_h_
#define _HTMLResourceReport_h_

#include <Report.h>

class HTMLResourceReport : public ReportHtml
{
public:
	HTMLResourceReport(Project* p, const QString& f, time_t s, time_t e) :
		ReportHtml(p, f, s, e)
	{
		// The default columns
		columns.append("resource");
		columns.append("daily");
	}
	~HTMLResourceReport() { }

	bool generate();

private:
	HTMLResourceReport() { }

	bool generateTableHeader();

	void dailyResourcePlan(Resource* r);
	void dailyTaskPlan(Resource* r, Task* t);
	void weeklyResourcePlan(Resource* r);
	void weeklyTaskPlan(Resource* r, Task* t);
	void monthlyResourcePlan(Resource* r);
	void monthlyTaskPlan(Resource* r, Task* t);
} ;

#endif
