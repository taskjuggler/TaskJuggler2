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

#include <qdict.h>

#include "Project.h"
#include "Utility.h"
#include "kotrus.h"

int Project::debugLevel = 0;

Project::Project()
{
	taskList.setAutoDelete(TRUE);
	resourceList.setAutoDelete(TRUE);
	activeAsap.setSorting(CoreAttributesList::PrioDown);
	activeAlap.setSorting(CoreAttributesList::PrioDown);
	priority = 500;
	dailyWorkingHours = 8.0;
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

	taskList.createIndex();
	resourceList.createIndex();
	accountList.createIndex();

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

	bool hasActualValues = FALSE;
	for (Task* t = taskList.first(); t != 0; t = taskList.next())
	{
		if (!t->preScheduleOk())
			error = TRUE;
		if (t->hasActualValues())
			hasActualValues = TRUE;
	}

	if (error)
		return FALSE;

	if (debugLevel > 0)
		qWarning("Scheduling plan scenario...");
	preparePlan();
	schedule();
	finishPlan();

	if (hasActualValues)
	{
		if (debugLevel > 0)
			qWarning("Scheduling actual scenario...");
		prepareActual();
		schedule();
		finishActual();
	}

	return TRUE;
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
	sortedTasks.setSorting(CoreAttributesList::PrioDown);
	sortedTasks.sort();

	time_t timeDelta = scheduleGranularity;
	bool done = FALSE;
	time_t day;

	updateActiveTaskList(sortedTasks);
	for (day = start; !(activeAsap.isEmpty() && activeAlap.isEmpty()) &&
			 day >= start && day < end; day += timeDelta)
	{
		timeDelta = scheduleGranularity;
		do
		{
			done = TRUE;
			for (Task* t = activeAlap.first(); t != 0; t = activeAlap.next())
			{
				if (!t->schedule(day, scheduleGranularity))
				{
					done = FALSE;
					break;	// Start with top priority tasks again.
				}
				if (t->needsEarlierTimeSlot(day + scheduleGranularity))
					timeDelta = -scheduleGranularity;
			}
		} while (!done);

		if (timeDelta < 0)
			continue;
		uint i = 0;
		do
		{
			done = TRUE;
			i = 0;
			for (Task* t = activeAsap.first(); t != 0; t = activeAsap.next())
			{
				i++;
				if (!t->schedule(day, scheduleGranularity))
				{
					done = FALSE;
					break;	// Start with top priority tasks again.
				}
			}
		} while (!done);
		if (i != activeAsap.count())
			qFatal("activeAsap list corrupted");
	}

	if (!activeAsap.isEmpty() || !activeAlap.isEmpty())
	{
		for (Task* t = activeAsap.first(); t != 0; t = activeAsap.next())
			qWarning("Task %s does not fit into the project time frame",
					 t->getId().latin1());
		for (Task* t = activeAlap.first(); t != 0; t = activeAlap.next())
			qWarning("Task %s does not fit into the project time frame",
					 t->getId().latin1());
		error = TRUE;
	}

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
	TaskList tl = taskList;
	tl.setSorting(CoreAttributesList::StartDown);
	tl.sort();
	int errors = 0;
	for (Task* t = tl.first(); t != 0; t = tl.next())
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
	if (debugLevel > 0)
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

	for (ExportReport* e = exportReports.first(); e != 0;
		 e = exportReports.next())
		e->generate();

	if( xmlreport )
	   xmlreport->generate();
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

	if (debugLevel > 2)
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
			if (debugLevel > 2)
				qWarning("Activating %s", t->getId().latin1());
			activeAsap.inSort(t);
		}
	}
	else
	{
		if (activeAlap.findRef(t) == -1)
		{
			if (debugLevel > 2)
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

