/*
 * ExportReport.cpp - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <qfile.h>

#include "Project.h"
#include "ExportReport.h"
#include "ExpressionTree.h"

ExportReport::ExportReport(Project* p, const QString& f) :
	Report(p, f, p->getStart(), p->getEnd())
{
	// show all tasks
	hideTask = new ExpressionTree(new Operation(0));
	// hide all resources
	hideResource = new ExpressionTree(new Operation(1));

	taskSortCriteria = CoreAttributesList::TreeMode;
	resourceSortCriteria = CoreAttributesList::TreeMode;
}

bool
ExportReport::generate()
{
	QFile f(fileName);
	if (!f.open(IO_WriteOnly))
	{
		qWarning("Cannot open export file %s!\n",
				 fileName.latin1());
		return FALSE;
	}
	s.setDevice(&f);
	
	TaskList filteredList;
	filterTaskList(filteredList, 0);
	sortTaskList(filteredList);

	for (Task* t = filteredList.first(); t != 0; t = filteredList.next())
	{
		QString start = time2ISO(t->getPlanStart());
		for (uint i = 0; i < start.length(); i++)
			if (start[i] == ' ')
				start[i] = '-';
		QString end = time2ISO(t->getPlanEnd());
		for (uint i = 0; i < end.length(); i++)
			if (end[i] == ' ')
				end[i] = '-';

		s << "task " << t->getId() << " \"" << t->getName() << "\""
			<< " { start " << start
		   	<< " end " << end;
		if (showActual)
		{
			QString start = time2ISO(t->getActualStart());
			for (uint i = 0; i < start.length(); i++)
				if (start[i] == ' ')
					start[i] = '-';
			QString end = time2ISO(t->getActualEnd());
			for (uint i = 0; i < end.length(); i++)
				if (end[i] == ' ')
					end[i] = '-';
			s << "actualStart " << start
				<< " actualEnd " << end;
		}
		
		if (t->isMilestone())
			s << "milestone ";
		
		s << " }" << endl;
	}

	f.close();
	return TRUE;
}

