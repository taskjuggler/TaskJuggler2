/*
 * HTMLResourceReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <qfile.h>

#include "Project.h"
#include "ResourceList.h"
#include "HTMLResourceReport.h"
#include "ExpressionTree.h"

HTMLResourceReport::HTMLResourceReport(Project* p, const QString& f,
									   time_t s, time_t e, const QString& df,
									   int dl) :
	ReportHtml(p, f, s, e, df, dl)
{
	columns.append("name");

	showActual = FALSE;
	// hide all tasks
	hideTask = new ExpressionTree(new Operation(1));
	// show all resources
	hideResource = new ExpressionTree(new Operation(0));

	taskSortCriteria[0] = CoreAttributesList::TreeMode;
	taskSortCriteria[1] = CoreAttributesList::StartUp;
	taskSortCriteria[2] = CoreAttributesList::EndUp;
	resourceSortCriteria[0] = CoreAttributesList::TreeMode;
}

bool
HTMLResourceReport::generate()
{
	if (!open())
		return FALSE;
	reportHTMLHeader();

	if (!generateTableHeader())
		return FALSE;

	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList);
	sortResourceList(filteredResourceList);

	for (Resource* r = filteredResourceList.first(); r != 0;
		 r = filteredResourceList.next())
	{
		generatePlanResource(r, 0, filteredResourceList.at() + 1);
		if (showActual)
			generateActualResource(r, 0);

		TaskList filteredTaskList;
		filterTaskList(filteredTaskList, r);
		sortTaskList(filteredTaskList);

		for (Task* t = filteredTaskList.first(); t != 0;
			 t = filteredTaskList.next())
		{
			generatePlanTask(t, r, filteredTaskList.at() + 1);
			if (showActual)
				generateActualTask(t, r);
		}
	}
	reportHTMLFooter();

	f.close();
	return TRUE;
}
