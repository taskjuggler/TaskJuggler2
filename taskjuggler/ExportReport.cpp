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
	
	TaskList filteredTaskList;
	filterTaskList(filteredTaskList, 0);
	sortTaskList(filteredTaskList);

	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList, 0);
	sortResourceList(filteredResourceList);

	generateTaskList(filteredTaskList, filteredResourceList);
	generateResourceList(filteredTaskList, filteredResourceList);
	
	f.close();
	return TRUE;
}

bool
ExportReport::generateTaskList(TaskList& filteredTaskList,
							   ResourceList& filteredResourceList)
{
	for (Task* t = filteredTaskList.first(); t != 0;
		 t = filteredTaskList.next())
	{
		QString start = time2tjp(t->getPlanStart());
		QString end = time2tjp(t->getPlanEnd());

		s << "task " << t->getId() << " \"" << t->getName() << "\""
			<< " { start " << start
			<< " end " << end;
		if (showActual)
		{
			QString start = time2tjp(t->getActualStart());
			QString end = time2tjp(t->getActualEnd());
			s << "actualStart " << start
				<< " actualEnd " << end;
		}

		if (!filteredResourceList.isEmpty())
			s << " projectid " << t->getProjectId() << " ";
		if (t->isMilestone())
			s << "milestone ";
		
		s << " }" << endl;
	}

	return TRUE;
}

bool
ExportReport::generateResourceList(TaskList& filteredTaskList,
								   ResourceList& filteredResourceList)
{
	for (Resource* r = filteredResourceList.first(); r != 0;
		 r = filteredResourceList.next())
	{
		s << "supplement resource " << r->getId() << " {" << endl;
		BookingList bl = r->getPlanJobs();
		bl.setAutoDelete(TRUE);
		for (Booking* b = bl.first(); b != 0; b = bl.next())
		{
			if (filteredTaskList.getTask(b->getTask()->getId()))
			{
				QString start = time2tjp(b->getStart());
				QString end = time2tjp(b->getEnd());
				s << "  planbooking " << start << " " << end 
					<< " " << b->getTask()->getId() << endl;
			}
		}
		bl = r->getActualJobs();
		bl.setAutoDelete(TRUE);
		for (Booking* b = bl.first(); b != 0; b = bl.next())
		{
			if (filteredTaskList.getTask(b->getTask()->getId()))
			{
				QString start = time2tjp(b->getStart());
				QString end = time2tjp(b->getEnd());
				s << "  actualbooking " << start << " " << end 
					<< " " << b->getTask()->getId() << endl;
			}
		}
		s << "}" << endl;
	}

	return TRUE;
}

