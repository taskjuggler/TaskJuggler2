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
#include <qmap.h>

#include "Project.h"
#include "ExportReport.h"
#include "ExpressionTree.h"

#define KW(a) a

static QMap<QString, int> TaskAttributeDict;
typedef enum TADs { TA_FLAGS = 0, TA_NOTE, TA_PRIORITY, TA_MINSTART,
	TA_MAXSTART, TA_MINEND, TA_MAXEND, TA_COMPLETE, TA_RESPONSIBLE,
	TA_DEPENDS };

ExportReport::ExportReport(Project* p, const QString& f,
						   const QString& df, int dl) :
	Report(p, f, p->getStart(), p->getEnd(), df, dl)
{
	if (TaskAttributeDict.empty())
	{
		TaskAttributeDict[KW("complete")] = TA_COMPLETE;
		TaskAttributeDict[KW("depends")] = TA_DEPENDS;
		TaskAttributeDict[KW("flags")] = TA_FLAGS;
		TaskAttributeDict[KW("maxend")] = TA_MAXEND;
		TaskAttributeDict[KW("maxstart")] = TA_MAXSTART;
		TaskAttributeDict[KW("minend")] = TA_MINEND;
		TaskAttributeDict[KW("minstart")] = TA_MINSTART;
		TaskAttributeDict[KW("note")] = TA_NOTE;
		TaskAttributeDict[KW("priority")] = TA_PRIORITY;
		TaskAttributeDict[KW("responsible")] = TA_RESPONSIBLE;
	}
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
	if (!open())
		return FALSE;
	
	TaskList filteredTaskList;
	filterTaskList(filteredTaskList, 0);
	sortTaskList(filteredTaskList);

	ResourceList filteredResourceList;
	filterResourceList(filteredResourceList, 0);
	sortResourceList(filteredResourceList);

	generateTaskList(filteredTaskList, filteredResourceList);
	generateTaskAttributeList(filteredTaskList);
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
		QString start = time2rfc(t->getPlanStart());
		QString end = time2rfc(t->getPlanEnd());

		s << "task " << t->getId() << " \"" << t->getName() << "\""
			<< " { start " << start
			<< " end " << end;
		if (showActual)
		{
			QString start = time2rfc(t->getActualStart());
			QString end = time2rfc(t->getActualEnd());
			s << "actualStart " << start
				<< " actualEnd " << end;
		}

		if (!filteredResourceList.isEmpty())
			s << " projectid " << t->getProjectId() << " ";
		if (t->isMilestone())
			s << " milestone ";
		
		s << " }" << endl;
	}

	return TRUE;
}

bool
ExportReport::generateTaskAttributeList(TaskList& filteredTaskList)
{
	if (taskAttributes.isEmpty())
		return TRUE;

	if (taskAttributes.contains("flags"))
	{
		FlagList allFlags;
		for (Task* t = filteredTaskList.first(); t != 0;
			 t = filteredTaskList.next())
		{
			QStringList fl = t->getFlagList();
			for (QStringList::Iterator jt = fl.begin();
				 jt != fl.end(); ++jt)
			{
				if (allFlags.find(*jt) == allFlags.end())
					allFlags.append(*jt);
			}

		}
		if (allFlags.begin() != allFlags.end())
		{
			s << "flags ";
			for (QStringList::Iterator it = allFlags.begin();
				 it != allFlags.end(); ++it)
			{
				if (it != allFlags.begin())
					s << ", ";
				s << *it;
			}
			s << endl;
		}
	}

	for (Task* t = filteredTaskList.first(); t != 0;
		 t = filteredTaskList.next())
	{
		s << "supplement task " << t->getId() << " {" << endl;
		for (QStringList::Iterator it = taskAttributes.begin(); 
			 it != taskAttributes.end(); ++it)
		{
			switch (TaskAttributeDict[*it])
			{
				case TA_FLAGS:
					{
						if (t->getFlagList().empty())
							break;
						s << "  flags ";
						QStringList fl = t->getFlagList();
						bool first = TRUE;
						for (QStringList::Iterator jt = fl.begin();
							 jt != fl.end(); ++jt)
						{
							if (!first)
								s << ", ";
							else
								first = FALSE;
							s << *jt;
						}
						s << endl;
						break;
					}
				case TA_NOTE:
					if (t->getNote() != "")
						s << "  note \"" << t->getNote() << "\"" << endl;
					break;
				case TA_MINSTART:
					if (t->getMinStart() != 0)
						s << "  minstart " << time2rfc(t->getMinStart()) 
							<< endl;
					break;
				case TA_MAXSTART:
					if (t->getMaxStart() != 0)
						s << "  maxstart " << time2rfc(t->getMaxStart()) 
							<< endl;
					break;
				case TA_MINEND:
					if (t->getMinEnd() != 0)
						s << "  minend " << time2rfc(t->getMinEnd()) << endl;
					break;
				case TA_MAXEND:
					if (t->getMaxEnd() != 0)
						s << "  maxend " << time2rfc(t->getMaxEnd()) << endl;
					break;
				case TA_COMPLETE:
					if (t->getComplete() >= 0.0)
						s << "  complete " << (int) t->getComplete() << endl;
					break;
				case TA_RESPONSIBLE:
					if (t->getResponsible())
						s << "  responsible " << t->getResponsible()->getId()
						   	<< endl;
					break;
				case TA_DEPENDS:
					if (t->firstPrevious())
					{
						s << "  depends ";
						bool first = TRUE;
						for (Task* tp = t->firstPrevious(); tp != 0;
							 tp = t->nextPrevious())
						{
							if (!first)
								s << ", ";
							else
								first = FALSE;
							s << tp->getId();
						}
						s << endl;
					}
					break;
				default:
					return FALSE;
			}
		}
		s << "}" << endl;
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
		BookingList bl = r->getPlanJobs();
		if (bl.isEmpty())
			continue;
		s << "supplement resource " << r->getId() << " {" << endl;
		bl.setAutoDelete(TRUE);
		for (Booking* b = bl.first(); b != 0; b = bl.next())
		{
			if (filteredTaskList.getTask(b->getTask()->getId()))
			{
				QString start = time2rfc(b->getStart());
				QString end = time2rfc(b->getEnd());
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
				QString start = time2rfc(b->getStart());
				QString end = time2rfc(b->getEnd());
				s << "  actualbooking " << start << " " << end 
					<< " " << b->getTask()->getId() << endl;
			}
		}
		s << "}" << endl;
	}

	return TRUE;
}

bool
ExportReport::addTaskAttribute(const QString& ta)
{
	/* Make sure the 'ta' is a valid attribute name and that we don't
	 * insert it twice into the list. Trying to insert it twice it not an
	 * error though. */
	if (TaskAttributeDict.find(ta) == TaskAttributeDict.end())
		return FALSE;

	if (taskAttributes.findIndex(ta) >= 0)
		return TRUE;
	taskAttributes.append(ta);
	return TRUE;
}

