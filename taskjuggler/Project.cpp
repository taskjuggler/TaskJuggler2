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

	vacationList.setAutoDelete(TRUE);
	
	htmlTaskReports.setAutoDelete(TRUE);
	htmlResourceReports.setAutoDelete(TRUE);
	htmlAccountReports.setAutoDelete(TRUE);
	htmlWeeklyCalendars.setAutoDelete(TRUE);
	exportReports.setAutoDelete(TRUE);
	
	xmlreport = 0;
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	icalReport = 0;
#endif
#endif

	priority = 500;
	/* The following settings are country and culture dependent. Those
	 * defaults are probably true for many Western countries, but have to be
	 * changed in project files. */
	dailyWorkingHours = 8.0;
	yearlyWorkingDays = 252;
	scheduleGranularity = ONEHOUR;
	weekStartsMonday = TRUE;
	timeFormat = "%Y-%m-%d %H:%M";
	shortTimeFormat = "%H:%M";

	start = 0;
	end = 0;
	now = time(0);
	
	minEffort = 0.0;
	maxEffort = 1.0;
	rate = 0.0;
	currencyDigits = 3;
	kotrus = 0;
	
	/* Initialize working hours with default values that match the Monday -
	 * Friday 9 - 6 (with 1 hour lunch break) pattern used by many western
	 * countries. */
	// Sunday
	workingHours[0] = new QPtrList<Interval>();
	workingHours[0]->setAutoDelete(TRUE);

	for (int i = 1; i < 6; ++i)
	{
		workingHours[i] = new QPtrList<Interval>();
		workingHours[i]->setAutoDelete(TRUE);
		workingHours[i]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
		workingHours[i]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));
	}

	// Saturday
	workingHours[6] = new QPtrList<Interval>();
	workingHours[6]->setAutoDelete(TRUE);
}

Project::~Project()
{
	delete xmlreport;
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	delete icalReport;
#endif
#endif
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
Project::pass2(bool checkOnlySyntax)
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
		if (!hasActualValues && t->hasActualValues())
			hasActualValues = TRUE;
	}

	for (Task* t = taskList.first(); t != 0; t = taskList.next())
		if (t->loopDetector())
			return FALSE;

	if (error)
		return FALSE;

	if (checkOnlySyntax)
		return TRUE;

	if (DEBUGPS(1))
		qWarning("Scheduling plan scenario...");
	preparePlan();
	if (!schedule("Plan"))
	{
		if (DEBUGPS(2))
			qWarning("Scheduling errors in plan scenario.");
		error = TRUE;
	}
	finishPlan();

	if (hasActualValues)
	{
		if (DEBUGPS(1))
			qWarning("Scheduling actual scenario...");
		prepareActual();
		if (!schedule("Actual"))
		{
			if (DEBUGPS(2))
				qWarning("Scheduling errors in actual scenario.");
			error = TRUE;
		}
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
Project::schedule(const QString& scenario)
{
	bool error = FALSE;

	TaskList sortedTasks(taskList);
	sortedTasks.setSorting(CoreAttributesList::PrioDown, 0);
	sortedTasks.setSorting(CoreAttributesList::SequenceUp, 1);
	sortedTasks.sort();

	bool done;
	do
	{
		done = TRUE;
		time_t slot = 0;
		for (Task* t = sortedTasks.first(); t; t = sortedTasks.next())
		{
			if (slot == 0)
			{
				slot = t->nextSlot(scheduleGranularity);
				if (slot == 0)
					continue;
				if (DEBUGPS(5))
					qWarning("Task %s requests slot %s", t->getId().latin1(),
							 time2ISO(slot).latin1());
				if (slot < start || slot > end)
				{
					t->setRunaway();
					if (DEBUGPS(5))
						qDebug("Marking task %s as runaway",
							   t->getId().latin1());
					error = TRUE;
				}
			}
			t->schedule(slot, scheduleGranularity);
			done = FALSE;
		}
	} while (!done);
	
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

	if (!checkSchedule(scenario))
		error = TRUE;

	return !error;
}

bool
Project::checkSchedule(const QString& scenario)
{
	int errors = 0;
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		/* Only check top-level tasks, since they recursively check their sub
		 * tasks. */
		if (t->getParent() == 0)
			t->scheduleOk(errors, scenario);
		if (errors >= 10)
		{
			qWarning(QString("Too many errors in %1 scenario. Giving up.")
					 .arg(scenario.lower()));
			break;
		}
	}

	return errors == 0;
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
Project::setKotrus(Kotrus* k)
{
	if (kotrus)
		delete kotrus;
	kotrus = k;
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
   pass2(FALSE);
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
      else if( tagName == "Project" )
      {
	 QString prjId = elem.attribute("Id");
	 addId( prjId );  // FIXME ! There can be more than one project ids!

	 prjId = elem.attribute("WeekStart");
	 setWeekStartsMonday( prjId == "Mon" );
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
