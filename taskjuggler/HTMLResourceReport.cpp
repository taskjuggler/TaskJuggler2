/*
 * HTMLResourceReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
#include "Operation.h"

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

    s << "<tbody>" << endl;
    
	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
	sortResourceList(filteredResourceList);

	int rNo = 1;
	for (ResourceListIterator rli(filteredResourceList); *rli != 0; 
		 ++rli, ++rNo)
	{
		generatePlanResource(*rli, 0, rNo);
		if (showActual)
			generateActualResource(*rli, 0);

		TaskList filteredTaskList;
		filterTaskList(filteredTaskList, *rli, hideTask, rollUpResource);
		sortTaskList(filteredTaskList);

		int tNo = 1;
		for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli, ++tNo)
		{
			generatePlanTask(*tli, *rli, tNo);
			if (showActual)
				generateActualTask(*tli, *rli);
		}
	}
    s << "</tbody>" << endl;
	s << "</table>" << endl;
	reportHTMLFooter();

	f.close();
	return TRUE;
}
