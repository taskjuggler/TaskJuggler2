/*
 * Project.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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

#include "Project.h"
#include "debug.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Scenario.h"
#include "Shift.h"
#include "Account.h"
#include "Resource.h"
#include "Utility.h"
#include "kotrus.h"

DebugController DebugCtrl;

Project::Project()
{
	taskList.setAutoDelete(TRUE);
	resourceList.setAutoDelete(TRUE);
	accountList.setAutoDelete(TRUE);
	shiftList.setAutoDelete(TRUE);

	vacationList.setAutoDelete(TRUE);
	
	htmlTaskReports.setAutoDelete(TRUE);
	htmlResourceReports.setAutoDelete(TRUE);
	htmlAccountReports.setAutoDelete(TRUE);
	htmlWeeklyCalendars.setAutoDelete(TRUE);
	exportReports.setAutoDelete(TRUE);

	Scenario* plan = new Scenario(this, "plan", "Plan", 0);
	new Scenario(this, "actual", "Actual", plan);
	scenarioList.createIndex(TRUE);
	scenarioList.createIndex(FALSE);

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
	yearlyWorkingDays = 260.714;
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

const QString&
Project::getScenarioName(int sc)
{
	return scenarioList.at(sc)->getName();
}

const QString&
Project::getScenarioId(int sc)
{
	return scenarioList.at(sc)->getId();
}

int
Project::getScenarioIndex(const QString& id)
{
	return scenarioList.getIndex(id);
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

void 
Project::addScenario(Scenario* s)
{
	scenarioList.append(s);
}

void 
Project::addTask(Task* t)
{
	taskList.append(t);
}

void
Project::addShift(Shift* s)
{
	shiftList.append(s);
}

void 
Project::addResource(Resource* r)
{
	resourceList.append(r);
}

void 
Project::addAccount(Account* a)
{
	accountList.append(a);
}

int
Project::calcWorkingDays(const Interval& iv) const
{
	int workingDays = 0;

	for (time_t s = midnight(iv.getStart()); s <= iv.getEnd(); 
		 s = sameTimeNextDay(s))
		if (isWorkingDay(s))
			workingDays++;

	return workingDays;
}

bool
Project::pass2(bool noDepCheck)
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
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
	{
		idHash.insert((*tli)->getId(), *tli);
	}
	// Create cross links from dependency lists.
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
	{
		if (!(*tli)->xRef(idHash))
			error = TRUE;
	}
	// Set dates according to implicit dependencies
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		(*tli)->implicitXRef();

	// Find out what scenarios need to be scheduled.
	// TODO: No multiple scenario support yet.
	hasExtraValues = FALSE;
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		if ((*tli)->hasExtraValues(Task::Actual))
		{
			hasExtraValues = TRUE;
			break;
		}

	/* Now we can copy the missing values from the plan scenario to the	other
	 * scenarios. */
	for (int sc = 1; sc < getMaxScenarios(); sc++)
		overlayScenario(sc);

	// Now check that all tasks have sufficient data to be scheduled.
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		if (!(*tli)->preScheduleOk())
			error = TRUE;

	if (!noDepCheck)
	{
		if (DEBUGPS(1))
			qDebug("Searching for dependency loops...");
		// Check all tasks for dependency loops.
		for (TaskListIterator tli(taskList); *tli != 0; ++tli)
			(*tli)->initLoopDetector();
		for (TaskListIterator tli(taskList); *tli != 0; ++tli)
			if ((*tli)->loopDetector())
				return FALSE;
	}

	return !error;
}

bool
Project::scheduleAllScenarios()
{
	bool error = FALSE;

	if (DEBUGPS(1))
		qDebug("Scheduling plan scenario...");
	prepareScenario(Task::Plan);
	if (!schedule("Plan"))
	{
		if (DEBUGPS(2))
			qDebug("Scheduling errors in plan scenario.");
		error = TRUE;
	}
	finishScenario(Task::Plan);

	if (hasExtraValues)
	{
		if (DEBUGPS(1))
			qDebug("Scheduling actual scenario...");
		prepareScenario(Task::Actual);
		if (!schedule("Actual"))
		{
			if (DEBUGPS(2))
				qDebug("Scheduling errors in actual scenario.");
			error = TRUE;
		}
		finishScenario(Task::Actual);
	}

	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		(*tli)->computeBuffers();

	/* Create indices for all lists according to their default sorting
	 * criteria. */
	taskList.createIndex();
	resourceList.createIndex();
	accountList.createIndex();
	shiftList.createIndex();

	return !error;	
}

void
Project::overlayScenario(int sc)
{
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		(*tli)->overlayScenario(sc);
}

void
Project::prepareScenario(int sc)
{
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		(*tli)->prepareScenario(sc);
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		(*tli)->propagateInitialValues();
	for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
		(*rli)->prepareScenario(sc);
}

void
Project::finishScenario(int sc)
{
	for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
		(*rli)->finishScenario(sc);
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
		(*tli)->finishScenario(sc);
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
		for (TaskListIterator tli(sortedTasks); *tli != 0; ++tli)
		{
			if (slot == 0)
			{
				slot = (*tli)->nextSlot(scheduleGranularity);
				if (slot == 0)
					continue;
				if (DEBUGPS(5))
					qDebug("Task %s requests slot %s", (*tli)->getId().latin1(),
							 time2ISO(slot).latin1());
				if (slot < start || 
					slot > (end - (time_t) scheduleGranularity + 1))
				{
					(*tli)->setRunaway();
					if (DEBUGPS(5))
						qDebug("Marking task %s as runaway",
							   (*tli)->getId().latin1());
					error = TRUE;
					slot = 0;
					continue;
				}
			}
			(*tli)->schedule(slot, scheduleGranularity);
			done = FALSE;
		}
	} while (!done);
	
	if (error)
		for (TaskListIterator tli(sortedTasks); *tli != 0; ++tli)
			if ((*tli)->isRunaway())
				if ((*tli)->getScheduling() == Task::ASAP)
					(*tli)->errorMessage
						(i18n("End of task %1 does not fit into the project "
							  "time frame.").arg((*tli)->getId()));
				else
					(*tli)->errorMessage
						(i18n("Start of task %1 does not fit into the "
							  "project time frame.").arg((*tli)->getId()));

	if (!checkSchedule(scenario))
		error = TRUE;

	return !error;
}

bool
Project::checkSchedule(const QString& scenario) const
{
	int errors = 0;
	for (TaskListIterator tli(taskList); *tli != 0; ++tli)
	{
		/* Only check top-level tasks, since they recursively check their sub
		 * tasks. */
		if ((*tli)->getParent() == 0)
			(*tli)->scheduleOk(errors, scenario);
		if (errors >= 10)
		{
			TJMH.errorMessage
				(i18n("Too many errors in %1 scenario. Giving up.")
				 .arg(scenario.lower()));
			break;
		}
	}

	return errors == 0;
}

void
Project::generateReports() const
{
	if (DEBUGPS(1))
		qDebug("Generating reports...");

	// Generate task reports
	for (QPtrListIterator<HTMLTaskReport> ri(htmlTaskReports); *ri != 0; ++ri)
		(*ri)->generate();

	// Generate resource reports
	for (QPtrListIterator<HTMLResourceReport> ri(htmlResourceReports); 
		 *ri != 0; ++ri)
		(*ri)->generate();

	// Generate account reports
	for (QPtrListIterator<HTMLAccountReport> ri(htmlAccountReports); 
		 *ri != 0; ++ri)
		(*ri)->generate();

	// Generate calendar reports
	for (QPtrListIterator<HTMLWeeklyCalendar> ri(htmlWeeklyCalendars); 
		 *ri != 0; ++ri)
		(*ri)->generate();

	// Generate status reports
	for (QPtrListIterator<HTMLStatusReport> ri(htmlStatusReports); 
		 *ri != 0; ++ri)
		(*ri)->generate();

	// Generate export files
	for (QPtrListIterator<ExportReport> ri(exportReports); *ri != 0; ++ri)
		(*ri)->generate();

	if( xmlreport )
	   xmlreport->generate();
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	if( icalReport )
	   icalReport->generate();
#endif
#endif

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
	
	for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)	
		(*rli)->dbLoadBookings((*rli)->getKotrusId(), 0);

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
   pass2(TRUE);
   scheduleAllScenarios();
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
      }
      else if( tagName == "Name" )
      {
	 setName( elem.text() );
      }
      else if( tagName == "Project" )
      {
	 QString prjId = elem.attribute("Id");
	 addId( prjId );  // FIXME ! There can be more than one project id!

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
