/*
 * HTMLStatusReport.cpp - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLStatusReport.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Project.h"
#include "Resource.h"
#include "Utility.h"

HTMLStatusReport::HTMLStatusReport(Project* p, const QString& f, time_t s,
									   time_t e, const QString& df, int dl) :
	ReportHtml(p, f, s, e, df, dl)
{
	// show all tasks
	hideTask = new ExpressionTree(new Operation(0));
	// hide all resources
	hideResource = new ExpressionTree(new Operation(1));

	taskSortCriteria[0] = CoreAttributesList::TreeMode;
	taskSortCriteria[1] = CoreAttributesList::StartUp;
	taskSortCriteria[2] = CoreAttributesList::EndUp;

	resourceSortCriteria[0] = CoreAttributesList::TreeMode;
	resourceSortCriteria[1] = CoreAttributesList::NameUp;
	resourceSortCriteria[2] = CoreAttributesList::IdUp;
}

bool
HTMLStatusReport::generate()
{
	if (!open())
		return FALSE;
	reportHTMLHeader();

	// Late tasks
	s << "<h3>Tasks that should have been finished already</h3>" << endl;
	TaskList filteredTaskList;
	Operation* op;
	op = new Operation
		(new Operation("istaskstatus", 
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::String, "inprogresslate", 0)),
		 Operation::And,
		 new Operation("endsbefore",
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::Date, project->getNow())));
	op = new Operation(op, Operation::Not);
	ExpressionTree* et = new ExpressionTree(op);

	start = project->getStart();
	end = project->getNow();
	
	filterTaskList(filteredTaskList, 0, et, rollUpTask);
	sortTaskList(filteredTaskList);
	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
	sortResourceList(filteredResourceList);
	
	if (filteredTaskList.count() == 0)
		s << "<p>None.</p>" << endl;
	else
	{
		columns.clear();
		columns.append("name");
		columns.append("duration");
		columns.append("end");
		columns.append("completed");
		columns.append("follows");
		columns.append("statusnote");

		generateTable(filteredTaskList, filteredResourceList);
	}
	delete et;
	s << "<br>" << endl;

	// In trouble tasks
	s << "<h3>Tasks that are behind schedule</h3>" << endl;
	op = new Operation
		(new Operation("istaskstatus", 
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::String, "inprogresslate", 0)),
		 Operation::And,
		 new Operation("endsafter",
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::Date, project->getNow())));
	op = new Operation(op, Operation::Not);
	et = new ExpressionTree(op);
	
	start = project->getStart();
	end = project->getEnd();	

	filterTaskList(filteredTaskList, 0, et, rollUpTask);
	sortTaskList(filteredTaskList);
	filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
	sortResourceList(filteredResourceList);

	if (filteredTaskList.count() == 0)
		s << "<p>None.</p>" << endl;
	else
	{
		columns.clear();
		columns.append("name");
		columns.append("duration");
		columns.append("end");
		columns.append("completed");
		columns.append("statusnote");

		generateTable(filteredTaskList, filteredResourceList);
	}
	delete et;
	s << "<br>" << endl;
	
	// Completed tasks
	s << "<h3>Tasks that have been completed</h3>" << endl;
	op = new Operation("istaskstatus", 
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::String, "finished", 0));
	op = new Operation(op, Operation::Not);
	et = new ExpressionTree(op);
	
	start = sameTimeLastWeek(project->getNow());
	end = project->getNow();
	
	filterTaskList(filteredTaskList, 0, et, rollUpTask);
	sortTaskList(filteredTaskList);
	filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
	sortResourceList(filteredResourceList);
	
	if (filteredTaskList.count() == 0)
		s << "<p>None.</p>" << endl;
	else
	{
		columns.clear();
		columns.append("name");
		columns.append("start");
		columns.append("end");
		columns.append("note");

		generateTable(filteredTaskList, filteredResourceList);
	}
	delete et;
	s << "<br>" << endl;

	// Upcoming tasks	
	s << "<h3>Upcoming new tasks</h3>" << endl;
	op = new Operation
		(new Operation("startsafter", 
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::Date, project->getNow())),
		 Operation::And,
		 new Operation("startsbefore",
					   new Operation(Operation::String, "plan", 0),
					   new Operation(Operation::Date, 
									 sameTimeNextWeek(project->getNow()))));
	op = new Operation(op, Operation::Not);
	et = new ExpressionTree(op);
	
	start = project->getNow();
	end = sameTimeNextWeek(project->getNow());
	
	filterTaskList(filteredTaskList, 0, et, rollUpTask);
	sortTaskList(filteredTaskList);
	filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
	sortResourceList(filteredResourceList);
	
	if (filteredTaskList.count() == 0)
		s << "<p>None.</p>" << endl;
	else
	{
		columns.clear();
		columns.append("name");
		columns.append("start");
		columns.append("duration");
		columns.append("resources");
		columns.append("note");

		generateTable(filteredTaskList, filteredResourceList);
	}
	delete et;
	s << "<br>" << endl;

	reportHTMLFooter();

	f.close();
	return TRUE;
}

bool
HTMLStatusReport::generateTable(TaskList& filteredTaskList, 
								ResourceList& filteredResourceList)
{
	generateTableHeader();
	
	int tNo = 1;
	for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli, ++tNo)
	{
		generatePlanTask(*tli, 0, tNo);
		if (showActual)
			generateActualTask(*tli, 0);

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

	return TRUE;
}

