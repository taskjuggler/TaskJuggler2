/*
 * HTMLTaskReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
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
#include "HTMLTaskReport.h"
#include "ExpressionTree.h"

HTMLTaskReport::HTMLTaskReport(Project* p, const QString& f, time_t s,
							   time_t e, const QString& df, int dl) :
	ReportHtml(p, f, s, e, df, dl)
{
	columns.append("no");
	columns.append("name");
	columns.append("start");
	columns.append("end");

	// show all tasks
	hideTask = new ExpressionTree(new Operation(0));
	// hide all resources
	hideResource = new ExpressionTree(new Operation(1));

	taskSortCriteria[0] = CoreAttributesList::TreeMode;
	taskSortCriteria[1] = CoreAttributesList::PlanStartUp;
	taskSortCriteria[2] = CoreAttributesList::PlanEndUp;
	resourceSortCriteria[0] = CoreAttributesList::TreeMode;
}

bool
HTMLTaskReport::generate()
{
	if (!open())
		return FALSE;
	reportHTMLHeader();

	if (!generateTableHeader())
		return FALSE;

	TaskList filteredList;
	filterTaskList(filteredList, 0);
	sortTaskList(filteredList);

	for (Task* t = filteredList.first(); t != 0; t = filteredList.next())
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
	}
	reportHTMLFooter();

	f.close();
	return TRUE;
}
