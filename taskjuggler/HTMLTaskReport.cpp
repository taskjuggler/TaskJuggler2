/*
 * HTMLTaskReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
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
							   time_t e) :
	ReportHtml(p, f, s, e)
{
	columns.append("no");
	columns.append("name");
	columns.append("start");
	columns.append("end");
	showActual = FALSE;
	// show all tasks
	hideTask = new ExpressionTree(new Operation(0));
	// hide all resources
	hideResource = new ExpressionTree(new Operation(1));

	taskSortCriteria = CoreAttributesList::TreeMode;
	resourceSortCriteria = CoreAttributesList::TreeMode;
}

bool
HTMLTaskReport::generate()
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
	s << "<table border=\"0\" cellpadding=\"1\">\n" << endl;

	if (!generateTableHeader())
		return FALSE;

	TaskList filteredList;
	filterTaskList(filteredList, 0);
	sortTaskList(filteredList);

	// Create a task->index no. dictionary.
	int i = 1;
	QDict<int> idxDict;
	idxDict.setAutoDelete(TRUE);
	for (Task* t = filteredList.first(); t != 0;
		 t = filteredList.next(), ++i)
		idxDict.insert(t->getId(), new int(i));

	i = 1;
	for (Task* t = filteredList.first(); t != 0;
		 t = filteredList.next(), ++i)
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
	s << "</table>" << endl;
	reportHTMLFooter();

	f.close();
	return TRUE;
}












