/*
 * Project.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <qdom.h>
#include <qdict.h>

#include "debug.h"
#include "Project.h"
#include "Utility.h"
#include "kotrus.h"

int Project::debugLevel = 0;
int Project::debugMode = -1;

Project::Project()
{
	taskList.setAutoDelete(TRUE);
	resourceList.setAutoDelete(TRUE);
	accountList.setAutoDelete(TRUE);
	activeAsap.setSorting(CoreAttributesList::PrioDown, 0);
	activeAlap.setSorting(CoreAttributesList::PrioDown, 0);
	priority = 500;
	/* The following settings are country and culture dependent. Those
	 * defaults are probably true for many Western countries, but have to be
	 * changed in project files. */
	dailyWorkingHours = 8.0;
	yearlyWorkingDays = 252;
	scheduleGranularity = ONEHOUR;
	start = 0;
	end = 0;
	now = time(0);
	copyright = "";
	minEffort = 0.0;
	maxEffort = 1.0;
	rate = 0.0;
	currency = "";
	currencyDigits = 3;
	kotrus = 0;
	xmlreport = 0;
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	icalReport = 0;
#endif
#endif
	
	/* Initialize working hours with default values that match the Monday -
	 * Friday 9 - 6 (with 1 hour lunch break) pattern used by many western
	 * countries. */
	// Sunday
	workingHours[0] = new QPtrList<Interval>();
	workingHours[0]->setAutoDelete(TRUE);

	// Monday
	workingHours[1] = new QPtrList<Interval>();
	workingHours[1]->setAutoDelete(TRUE);
	workingHours[1]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
	workingHours[1]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));
	// Tuesday
	workingHours[2] = new QPtrList<Interval>();
	workingHours[2]->setAutoDelete(TRUE);
	workingHours[2]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
	workingHours[2]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));
	// Wednesday
	workingHours[3] = new QPtrList<Interval>();
	workingHours[3]->setAutoDelete(TRUE);
	workingHours[3]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
	workingHours[3]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));
	// Thursday
	workingHours[4] = new QPtrList<Interval>();
	workingHours[4]->setAutoDelete(TRUE);
	workingHours[4]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
	workingHours[4]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));
	// Friday
	workingHours[5] = new QPtrList<Interval>();
	workingHours[5]->setAutoDelete(TRUE);
	workingHours[5]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
	workingHours[5]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));

	// Saturday
	workingHours[6] = new QPtrList<Interval>();
	workingHours[6]->setAutoDelete(TRUE);
}

bool
Project::addId(const QString& id)
{
	if (projectIDs.findIndex(id) != -1)
		return FALSE;
	else
		projectIDs.append(id);
	return TRUE;
}

QString
Project::getIdIndex(const QString& i) const
{
	int idx;
	if ((idx = projectIDs.findIndex(i)) == -1)
		return QString("?");
	QString idxStr;
	do
	{
		idxStr = QChar('A' + idx % ('Z' - 'A')) + idxStr;
		idx /= 'Z' - 'A';
	} while (idx > 'Z' - 'A');

	return idxStr;
}

bool
Project::addTask(Task* t)
{
	taskList.append(t);
	return TRUE;
}

bool
Project::pass2()
{
	QDict<Task> idHash;
	bool error = FALSE;

	// Generate sequence numbers for all lists.
	taskList.createIndex(TRUE);
	resourceList.createIndex(TRUE);
	accountList.createIndex(TRUE);
	shiftList.createIndex(TRUE);

	// Initialize random generator.
	srand((int) start);
	
	// Create hash to map task IDs to pointers.
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		idHash.insert(t->getId(), t);
	}
	// Create cross links from dependency lists.
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		if (!t->xRef(idHash))
			error = TRUE;
	}
	// Set dates according to implicit dependencies
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->implicitXRef();

	bool hasActualValues = FALSE;
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		if (!t->preScheduleOk())
			error = TRUE;
		if (t->hasActualValues())
			hasActualValues = TRUE;
	}

	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		if (t->loopDetector())
			return FALSE;

	if (error)
		return FALSE;

	if (DEBUGPS(1))
		qWarning("Scheduling plan scenario...");
	preparePlan();
	if (!schedule())
		error = TRUE;
	finishPlan();

	if (hasActualValues)
	{
		if (DEBUGPS(1))
			qWarning("Scheduling actual scenario...");
		prepareActual();
		if (!schedule())
			error = TRUE;
		finishActual();
	}

	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->computeBuffers();

	/* Create indices for all lists according to their default sorting
	 * criteria. */
	taskList.createIndex();
	resourceList.createIndex();
	accountList.createIndex();
	shiftList.createIndex();
	
	return !error;
}

void
Project::preparePlan()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->preparePlan();
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->propagateInitialValues();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->preparePlan();
}

void
Project::finishPlan()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->finishPlan();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->finishPlan();
}

void
Project::prepareActual()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->prepareActual();
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->propagateInitialValues();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->prepareActual();
}

void
Project::finishActual()
{
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		t->finishActual();
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->finishActual();
}

bool
Project::schedule()
{
	bool error = FALSE;

	TaskList sortedTasks(taskList);
	sortedTasks.setSorting(CoreAttributesList::PrioDown, 0);
	sortedTasks.sort();

	time_t timeDelta = scheduleGranularity;
	bool done = FALSE;
	time_t day;

	/* Find all tasks than have enough information to be scheduled and sort
	 * them into two lists; one list for all ASAP tasks and one list for all
	 * ALAP tasks. */
	updateActiveTaskList(sortedTasks);
	for (day = start; !(activeAsap.isEmpty() && activeAlap.isEmpty()); )
	{
		if (DEBUGPS(10))
			qWarning("Scheduling slot %s", time2ISO(day).latin1());
		timeDelta = scheduleGranularity;
		do
		{
			if (DEBUGPS(10))
				qWarning("%d active ALAP tasks", activeAlap.count());
			done = TRUE;
			for (Task* t = activeAlap.first(); t != 0; t = activeAlap.next())
			{
				if (!t->schedule(day, scheduleGranularity))
				{
					done = FALSE;
					break;	// Start with top priority tasks again.
				}
				if (t->needsEarlierTimeSlot(day + scheduleGranularity))
				{
					if (DEBUGPS(5))
						qWarning("Scheduling backwards now");
					timeDelta = -scheduleGranularity;
				}
			}
		} while (!done);

		if (timeDelta >= 0)
		{
			do
			{
				if (DEBUGPS(10))
					qWarning("%d active ASAP tasks", activeAsap.count());
				done = TRUE;
				for (Task* t = activeAsap.first(); t != 0;
					 t = activeAsap.next())
				{
					if (!t->schedule(day, scheduleGranularity))
					{
						done = FALSE;
						break;	// Start with top priority tasks again.
					}
				}
			} while (!done);
		}
		day += timeDelta;
	
		/* Runaway detection */
		if (day < start)
		{
			if (DEBUGPS(2))
				qWarning("Scheduler ran over start of project");
			
			for (Task* t = activeAlap.first(); t != 0; t = activeAlap.next())
				if (t->setRunaway(day - timeDelta, scheduleGranularity))
				{
					if (DEBUGPS(5))
						qDebug("Marking task %s as runaway",
							   t->getId().latin1());
					error = TRUE;
				}
			day = start;
		}
		if (day >= end)
		{
			if (DEBUGPS(2))
				qDebug("Scheduler ran over end of project");
			
			for (Task* t = activeAsap.first(); t != 0; t = activeAsap.next())
				if (t->setRunaway(day - timeDelta, scheduleGranularity))
				{
					if (DEBUGPS(5))
						qDebug("Marking task %s as runaway",
							   t->getId().latin1());
					error = TRUE;
				}
			day = end - scheduleGranularity;
		}
	}

	if (error)
		for (Task* t = sortedTasks.first(); t; t = sortedTasks.next())
			if (t->isRunaway())
				if (t->getScheduling() == Task::ASAP)
					t->fatalError(QString(
						"End of task %1 does not fit into the project time "
						"frame.").arg(t->getId()));
				else
					t->fatalError(QString(
						"Start of task %1 does not fit into the project time "
						"frame.").arg(t->getId()));

	if (!checkSchedule())
		error = TRUE;

	return !error;
}

void
Project::updateActiveTaskList(TaskList& sortedTasks)
{
	for (Task* t = sortedTasks.first(); t != 0; t = sortedTasks.next())
		if (t->isActive())
			addActiveTask(t);
}

bool
Project::checkSchedule()
{
	int errors = 0;
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		if (!t->scheduleOk())
			errors++;
		if (errors >= 10)
			break;
	}

	return errors == 0;
}

void
Project::setKotrus(Kotrus* k)
{
	if (kotrus)
		delete kotrus;
	kotrus = k;
}

void
Project::generateReports()
{
	if (DEBUGPS(1))
		qWarning("Generating reports...");

	// Generate task reports
	for (HTMLTaskReport* h = htmlTaskReports.first(); h != 0;
		 h = htmlTaskReports.next())
		h->generate();

	// Generate resource reports
	for (HTMLResourceReport* r = htmlResourceReports.first(); r != 0;
		 r = htmlResourceReports.next())
		r->generate();

	// Generate account reports
	for (HTMLAccountReport* r = htmlAccountReports.first(); r != 0;
		 r = htmlAccountReports.next())
		r->generate();

	// Generate calendar reports
	for (HTMLWeeklyCalendar* r = htmlWeeklyCalendars.first(); r != 0;
		 r = htmlWeeklyCalendars.next())
		r->generate();

	// Generate export files
	for (ExportReport* e = exportReports.first(); e != 0;
		 e = exportReports.next())
		e->generate();

	if( xmlreport )
	   xmlreport->generate();
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	if( icalReport )
	   icalReport->generate();
#endif
#endif

}

bool
Project::needsActualDataForReports()
{
	bool needsActual = FALSE;

	// Generate task reports
	for (HTMLTaskReport* h = htmlTaskReports.first(); h != 0;
		 h = htmlTaskReports.next())
		if (h->getShowActual())
			needsActual = TRUE;
	// Generate resource reports
	for (HTMLResourceReport* h = htmlResourceReports.first(); h != 0;
		 h = htmlResourceReports.next())
		if (h->getShowActual())
			needsActual = TRUE;

	// Generate account reports
	for (HTMLAccountReport* h = htmlAccountReports.first(); h != 0;
		 h = htmlAccountReports.next())
		if (h->getShowActual())
			needsActual = TRUE;

	return needsActual;
}

void
Project::removeActiveTask(Task* t)
{
	t->setScheduled();

	if (DEBUGPS(5))
		qWarning("Deactivating %s", t->getId().latin1());

	if (t->getScheduling() == Task::ASAP)
		activeAsap.removeRef(t);
	else
		activeAlap.removeRef(t);
}

void
Project::addActiveTask(Task* t)
{
	if (t->getScheduling() == Task::ASAP)
	{
		if (activeAsap.findRef(t) == -1)
		{
			if (DEBUGPS(5))
				qWarning("Activating %s", t->getId().latin1());
			activeAsap.inSort(t);
		}
	}
	else
	{
		if (activeAlap.findRef(t) == -1)
		{
			if (DEBUGPS(5))
				qWarning("Activating %s", t->getId().latin1());	
			activeAlap.inSort(t);
		}
	}
}

bool
Project::readKotrus()
{
	if (!kotrus)
		return TRUE;
		
	for (Resource* r = resourceList.first(); r != 0; r = resourceList.next())
		r->dbLoadBookings(r->getKotrusId(), 0);

	return TRUE;
}

bool
Project::updateKotrus()
{
	return TRUE;
}


bool
Project::loadFromXML( const QString& inpFile )
{
   QDomDocument doc;
   QFile file( inpFile );

   doc.setContent( &file );
   qDebug(  "Loading XML " + inpFile );

   QDomElement elemProject = doc.documentElement();

   if( !elemProject.isNull())
   {
      parseDomElem( elemProject );
   }
   else
   {
      qDebug("Empty !" );
   }
   pass2();
   return true;
}


void Project::parseDomElem( QDomElement& parentElem )
{
   QDomElement elem = parentElem.firstChild().toElement();

   for( ; !elem.isNull(); elem = elem.nextSibling().toElement() )
   {
      QString tagName = elem.tagName();
      
      qDebug(  "|| elemType: " + tagName );
      
      if( tagName == "Task" )
      {
	 QString tId = elem.attribute("Id");
	 Task *t = new Task( this, tId, QString(), 0, QString(), 0 );

	 t->loadFromXML( elem, this  );
	 addTask( t );
      }
      else if( tagName == "Name" )
      {
	 setName( elem.text() );
      }
      else if( tagName == "Version" )
	 setVersion( elem.text() );
      else if( tagName == "Priority" )
	 setPriority( elem.text().toInt());
      else if( tagName == "start" )
	 setStart( elem.text().toLong());
      else if( tagName == "end" )
	 setEnd( elem.text().toLong());
	       
      // parseDomElem( elem );
   }
}
