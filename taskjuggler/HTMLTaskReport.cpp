/*
 * HTMLTaskReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
#include "Operation.h"

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
	taskSortCriteria[1] = CoreAttributesList::StartUp;
	taskSortCriteria[2] = CoreAttributesList::EndUp;
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
	filterTaskList(filteredList, 0, hideTask, rollUpTask);
	sortTaskList(filteredList);

	int tNo = 1;
	for (TaskListIterator tli(filteredList); *tli != 0; ++tli, ++tNo)
	{
		generatePlanTask(*tli, 0, tNo);
		if (showActual)
			generateActualTask(*tli, 0);

		ResourceList filteredResourceList;
		filterResourceList(filteredResourceList, *tli, hideResource,
						   rollUpResource);
		sortResourceList(filteredResourceList);

		int rNo = 1;
		for (ResourceListIterator rli(filteredResourceList); *rli != 0; 
			 ++rli, ++rNo)
		{
			generatePlanResource(*rli, *tli, rNo);
			if (showActual)
				generateActualResource(*rli, *tli);
		}
	}
	s << "</table>" << endl;
	reportHTMLFooter();

	f.close();
	return TRUE;
}
