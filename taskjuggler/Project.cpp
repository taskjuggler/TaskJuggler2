/*
 * Project.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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

#include "Interval.h"
#include "Project.h"
#include "debug.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Scenario.h"
#include "Shift.h"
#include "Account.h"
#include "Task.h"
#include "Resource.h"
#include "Utility.h"
#include "VacationInterval.h"
#include "Optimizer.h"
#include "OptimizerRun.h"
#include "HTMLTaskReport.h"
#include "HTMLResourceReport.h"
#include "HTMLAccountReport.h"
#include "HTMLWeeklyCalendar.h"
#include "HTMLStatusReport.h"
#include "CSVTaskReport.h"
#include "CSVResourceReport.h"
#include "CSVAccountReport.h"
#include "ExportReport.h"
#include "ReportXML.h"
#include "UsageLimits.h"
#include "kotrus.h"
#include "CustomAttributeDefinition.h"

DebugController DebugCtrl;

Project::Project()
{
    /* Pick some reasonable initial number since we don't know the
     * project time frame yet. */
    initUtility(20000);

    vacationList.setAutoDelete(TRUE);
    taskAttributes.setAutoDelete(TRUE);
    resourceAttributes.setAutoDelete(TRUE);
    reports.setAutoDelete(TRUE);

    allowRedefinitions = FALSE;

    currentId = QString::null;

    new Scenario(this, "plan", "Plan", 0);
    scenarioList.createIndex(TRUE);
    scenarioList.createIndex(FALSE);

    /* This is the version 1.0 XML reports. It should be deleted for the next
     * major release. */
    xmlreport = 0;

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
    numberFormat = RealFormat("-", "", ",", ".", 1);
    currencyFormat = RealFormat("(", ")", ",", ".", 0);

    start = 0;
    end = 0;
    now = time(0);

    minEffort = 0.0;
    resourceLimits = 0;
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

    maxErrors = 0;
}

Project::~Project()
{
    while (!taskList.isEmpty())
    {
        for (TaskListIterator tli(taskList); *tli; ++tli)
            if ((*tli)->getParent() == 0)
            {
                delete *tli;
                break;
            }
    }

    while (!resourceList.isEmpty())
    {
        for (ResourceListIterator tli(resourceList); *tli; ++tli)
            if ((*tli)->getParent() == 0)
            {
                delete *tli;
                break;
            }
    }
    Resource::deleteStaticData();

    while (!accountList.isEmpty())
    {
        for (AccountListIterator tli(accountList); *tli; ++tli)
            if ((*tli)->getParent() == 0)
            {
                delete *tli;
                break;
            }
    }

    while (!shiftList.isEmpty())
    {
        for (ShiftListIterator tli(shiftList); *tli; ++tli)
            if ((*tli)->getParent() == 0)
            {
                delete *tli;
                break;
            }
    }

    while (!scenarioList.isEmpty())
    {
        for (ScenarioListIterator tli(scenarioList); *tli; ++tli)
            if ((*tli)->getParent() == 0)
            {
                delete *tli;
                break;
            }
    }

    // Remove support for 1.0 XML reports for next major release. */
    delete xmlreport;

    for (int i = 0; i < 7; ++i)
        delete workingHours[i];
    exitUtility();
}

void
Project::addSourceFile(const QString& f)
{
    if (sourceFiles.find(f) == sourceFiles.end())
        sourceFiles.append(f);
}

QStringList
Project::getSourceFiles() const
{
    return sourceFiles;
}

void
Project::setProgressInfo(const QString& i)
{
    emit updateProgressInfo(i);
}

void
Project::setProgressBar(int i, int of)
{
    emit updateProgressBar(i, of);
}

void
Project::setTimeZone(const QString& tz)
{
    setTimezone(tz);
    timeZone = tz;
}

Scenario*
Project::getScenario(int sc) const
{
    int i = 0;
    for (ScenarioListIterator sli(scenarioList); sli; ++sli)
        if (i++ == sc)
            return *sli;
    return 0;
}

const QString&
Project::getScenarioName(int sc) const
{
    int i = 0;
    for (ScenarioListIterator sli(scenarioList); sli; ++sli)
        if (i++ == sc)
            return (*sli)->getName();

    return QString::null;
}

const QString&
Project::getScenarioId(int sc) const
{
    int i = 0;
    for (ScenarioListIterator sli(scenarioList); sli; ++sli)
        if (i++ == sc)
            return (*sli)->getId();

    return QString::null;
}

int
Project::getScenarioIndex(const QString& id) const
{
    return scenarioList.getIndex(id);
}

bool
Project::addId(const QString& id, bool changeCurrentId)
{
    if (projectIDs.findIndex(id) != -1)
        return FALSE;
    else
        projectIDs.append(id);

    if (changeCurrentId)
        currentId = id;

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

    /* This is not too efficient, but since there are usually only a few
     * scenarios in a project, this doesn't hurt too much. */
    scenarioList.createIndex(TRUE);
    scenarioList.createIndex(FALSE);
}

void
Project::deleteScenario(Scenario* s)
{
    scenarioList.removeRef(s);
}

void
Project::setResourceLimits(UsageLimits* l)
{
    if (resourceLimits)
        delete resourceLimits;
    resourceLimits = l;
}

void
Project::addTask(Task* t)
{
    taskList.append(t);
}

void
Project::deleteTask(Task* t)
{
    taskList.removeRef(t);
}

bool
Project::addTaskAttribute(const QString& id, CustomAttributeDefinition* cad)
{
    if (taskAttributes.find(id))
        return FALSE;

    taskAttributes.insert(id, cad);
    return TRUE;
}

const CustomAttributeDefinition*
Project::getTaskAttribute(const QString& id) const
{
    return taskAttributes[id];
}

void
Project::addShift(Shift* s)
{
    shiftList.append(s);
}

void
Project::deleteShift(Shift* s)
{
    shiftList.removeRef(s);
}

void
Project::addResource(Resource* r)
{
    resourceList.append(r);
}

void
Project::deleteResource(Resource* r)
{
    resourceList.removeRef(r);
}

bool
Project::addResourceAttribute(const QString& id,
                              CustomAttributeDefinition* cad)
{
    if (resourceAttributes.find(id))
        return FALSE;

    resourceAttributes.insert(id, cad);
    return TRUE;
}

const CustomAttributeDefinition*
Project::getResourceAttribute(const QString& id) const
{
    return resourceAttributes[id];
}

void
Project::addAccount(Account* a)
{
    accountList.append(a);
}

void
Project::deleteAccount(Account* a)
{
    accountList.removeRef(a);
}

bool
Project::addAccountAttribute(const QString& id,
                              CustomAttributeDefinition* cad)
{
    if (resourceAttributes.find(id))
        return FALSE;

    resourceAttributes.insert(id, cad);
    return TRUE;
}

const CustomAttributeDefinition*
Project::getAccountAttribute(const QString& id) const
{
    return resourceAttributes[id];
}

bool
Project::isWorkingDay(time_t d) const
{
    return !(workingHours[dayOfWeek(d, FALSE)]->isEmpty() ||
             isVacation(d));
}

bool
Project::isWorkingTime(time_t d) const
{
    if (isVacation(d))
        return FALSE;

    int dow = dayOfWeek(d, FALSE);
    for (QPtrListIterator<Interval> ili(*(workingHours[dow])); *ili != 0; ++ili)
    {
        if ((*ili)->contains(secondsOfDay(d)))
            return TRUE;
    }
    return FALSE;
}

bool
Project::isWorkingTime(const Interval& iv) const
{
    if (isVacation(iv.getStart()))
        return FALSE;

    int dow = dayOfWeek(iv.getStart(), FALSE);
    for (QPtrListIterator<Interval> ili(*(workingHours[dow])); *ili != 0; ++ili)
    {
        if ((*ili)->contains(Interval(secondsOfDay(iv.getStart()),
                                  secondsOfDay(iv.getEnd()))))
            return TRUE;
    }
    return FALSE;
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

double
Project::convertToDailyLoad(long secs) const
{
    return ((double) secs / (dailyWorkingHours * ONEHOUR));
}

void
Project::addJournalEntry(JournalEntry* entry)
{
    journal.append(entry);
}

JournalIterator
Project::getJournalIterator() const
{
    return JournalIterator(journal);
}

bool
Project::pass2(bool noDepCheck, bool& fatalError)
{
    if (taskList.isEmpty())
    {
        TJMH.errorMessage(i18n("The project does not contain any tasks."));
        return FALSE;
    }

    QDict<Task> idHash;
    bool error = FALSE;

    /* The optimum size for the localtime hash is twice the number of time
     * slots times 2 (because of timeslot and timeslot - 1s). */
    initUtility(4 * ((end - start) / scheduleGranularity));

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
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
    {
        // Set dates according to implicit dependencies
        (*tli)->implicitXRef();

        // Sort allocations properly
        (*tli)->sortAllocations();

        // Save so far booked resources as specified resources
        (*tli)->saveSpecifiedBookedResources();
    }

    // Save a copy of all manually booked resources.
    for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
        (*rli)->saveSpecifiedBookings();

    /* Now we can copy the missing values from the plan scenario to the other
     * scenarios. */
    if (scenarioList.count() > 1)
    {
        for (ScenarioListIterator sli(scenarioList[0]->getSubListIterator());
             *sli; ++sli)
            overlayScenario(0, (*sli)->getSequenceNo() - 1);
    }

    // Now check that all tasks have sufficient data to be scheduled.
    setProgressInfo(i18n("Checking scheduling data..."));
    for (ScenarioListIterator sci(scenarioList); *sci; ++sci)
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            if (!(*tli)->preScheduleOk((*sci)->getSequenceNo() - 1))
            {
                error = TRUE;
                fatalError = true;
            }

    if (!noDepCheck)
    {
        setProgressInfo(i18n("Searching for dependency loops ..."));
        if (DEBUGPS(1))
            qDebug("Searching for dependency loops ...");
        // Check all tasks for dependency loops.
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            (*tli)->initLoopDetector();
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            if ((*tli)->loopDetector())
            {
                fatalError = true;
                return FALSE;
            }

        setProgressInfo(i18n("Searching for underspecified tasks ..."));
        if (DEBUGPS(1))
            qDebug("Searching for underspecified tasks ...");
        for (ScenarioListIterator sci(scenarioList); *sci; ++sci)
            for (TaskListIterator tli(taskList); *tli != 0; ++tli)
                if (!(*tli)->checkDetermination((*sci)->getSequenceNo() - 1))
                {
                    error = TRUE;
                    fatalError = true;
                }
        if (fatalError)
            return false;
    }

    return !error;
}

bool
Project::scheduleScenario(Scenario* sc)
{
    bool error = FALSE;

    setProgressInfo(i18n("Scheduling scenario %1...").arg(sc->getId()));

    int scIdx = sc->getSequenceNo() - 1;
    prepareScenario(scIdx);

    if (!schedule(scIdx))
    {
        if (DEBUGPS(2))
            qDebug(i18n("Scheduling errors in scenario '%1'.")
                   .arg(sc->getId()));
        error = TRUE;
    }
    finishScenario(scIdx);

    for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
    {
        if (!(*rli)->bookingsOk(scIdx))
        {
            error = TRUE;
            break;
        }
    }

    return !error;
}

void
Project::completeBuffersAndIndices()
{
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->computeBuffers();

    /* Create indices for all lists according to their default sorting
     * criteria. */
    taskList.createIndex();
    resourceList.createIndex();
    accountList.createIndex();
    shiftList.createIndex();
}

bool
Project::scheduleAllScenarios()
{
    bool schedulingOk = TRUE;
    for (ScenarioListIterator sci(scenarioList); *sci; ++sci)
        if ((*sci)->getEnabled())
        {
            if (DEBUGPS(1))
                qDebug(i18n("Scheduling scenario '%1' ...")
                       .arg((*sci)->getId()));

            if (!scheduleScenario(*sci))
                schedulingOk = FALSE;
        }

    completeBuffersAndIndices();

    return schedulingOk;
}

void
Project::overlayScenario(int base, int sc)
{
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->overlayScenario(base, sc);

    for (ScenarioListIterator sli(scenarioList[base]->getSubListIterator());
         *sli; ++sli)
        overlayScenario(sc, (*sli)->getSequenceNo() - 1);
}

void
Project::prepareScenario(int sc)
{
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->prepareScenario(sc);

    /* First we compute the criticalness of the individual task without their
     * dependency context. */
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->computeCriticalness(sc);

    /* Then we compute the path criticalness that represents the criticalness
     * of a task taking their dependency context into account. */
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->computePathCriticalness(sc);

    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->propagateInitialValues(sc);

    for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
        (*rli)->prepareScenario(sc);
    if (DEBUGTS(4))
    {
        qDebug("Allocation probabilities for the resources:");
        for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
            qDebug("Resource %s: %f%%",
                   (*rli)->getId().latin1(),
                   (*rli)->getAllocationProbability(sc));
        qDebug("Criticalnesses of the tasks with respect to resource "
               "availability:");
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            qDebug("Task %s: %-5.1f %-5.1f", (*tli)->getId().latin1(),
                   (*tli)->getCriticalness(sc),
                   (*tli)->getPathCriticalness(sc));
    }
}

void
Project::finishScenario(int sc)
{
    for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
        (*rli)->finishScenario(sc);
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->finishScenario(sc);
    /* We need to have finished the scenario for all tasks before we can
     * calculate the completion degree. */
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->calcCompletionDegree(sc);
}

bool
Project::schedule(int sc)
{
    bool error = FALSE;

    TaskList fullSortedTasks(taskList);
    // The scheduling function only cares about leaf tasks. Container tasks
    // are scheduled automatically when all their childern are scheduled. So
    // we create a task list that only contains leaf tasks.
    TaskList sortedTasks;
    int leafTasks = 0;
    for (TaskListIterator tli(fullSortedTasks); *tli != 0; ++tli)
        if (!(*tli)->hasSubs())
        {
            sortedTasks.append(*tli);
            leafTasks++;
        }

    sortedTasks.setSorting(CoreAttributesList::PrioDown, 0);
    sortedTasks.setSorting(CoreAttributesList::PathCriticalnessDown, 1);
    sortedTasks.setSorting(CoreAttributesList::SequenceUp, 2);
    sortedTasks.sort();

    bool done;
    /* While the scheduling process progresses, the list contains more and
     * more scheduled tasks. We use the cleanupTimer to remove those in
     * certain intervals. As we potentially have already completed tasks in
     * the list when we start, we initialize the timer with a very large
     * number so the first round of cleanup is done right after the first
     * scheduling pass. */
    int cleanupTimer = 100000;
    do
    {
        done = TRUE;
        time_t slot = 0;
        int priority = 0;
        Task::SchedulingInfo schedulingInfo = Task::ASAP;

        /* The task list is sorted by priority. The priority decreases towards
         * the end of the list. We iterate through the list and look for a
         * task that can be scheduled. It the determines the time slot that
         * will be scheduled during this run for all subsequent tasks as well.
         */
        for (TaskListIterator tli(sortedTasks); *tli != 0; ++tli)
        {
            if (slot == 0)
            {
                /* No time slot has been set yet. Check if this task can be
                 * scheduled and provides a suggestion. */
                slot = (*tli)->nextSlot(sc, scheduleGranularity);
                priority = (*tli)->getPriority();
                schedulingInfo = (*tli)->getScheduling();
                /* If not, try the next task. */
                if (slot == 0)
                    continue;

                if (DEBUGPS(4))
                    qDebug("Task '%s' (Prio %d) requests slot %s",
                           (*tli)->getId().latin1(), (*tli)->getPriority(),
                           time2ISO(slot).latin1());
                /* If the task wants a time slot outside of the project time
                 * frame, we flag this task as a runaway and go to the next
                 * task. */
                if (slot < start ||
                    slot > (end - (time_t) scheduleGranularity + 1))
                {
                    (*tli)->setRunaway();
                    if (DEBUGPS(5))
                        qDebug("Marking task '%s' as runaway",
                               (*tli)->getId().latin1());
                    error = TRUE;
                    slot = 0;
                    continue;
                }
            }
            done = FALSE;
            /* Each task has a scheduling direction (forward or backward)
             * depending on it's constrains. The task with the highest
             * priority determins the time slot and hence the scheduling
             * direction. Since tasks that have the other direction cannot the
             * scheduled then, we have to stop this run as soon as we hit a
             * task that runs in the other direction. If we would not do this,
             * tasks with lower priority would grab resources form tasks with
             * higher priority. */
            if ((*tli)->getScheduling() != schedulingInfo &&
                !(*tli)->isMilestone())
            {
                if (DEBUGPS(4))
                    qDebug("Changing scheduling direction to %d due to task "
                           "'%s'", (*tli)->getScheduling(),
                           (*tli)->getId().latin1());
                break;
            }
            /* We must avoid that lower priority tasks get resources even
             * though there are higher priority tasks that are ready to be
             * scheduled but have a non-adjacent last slot. */
            if ((*tli)->getPriority() < priority)
                break;

            // Schedule this task for the current time slot.
            (*tli)->schedule(sc, slot, scheduleGranularity);
        }

        /* Remove all fully scheduled tasks from the list in regular
         * intervals. This is fairly expensive, so we need to balance the
         * frequency with the performance improvements due to the shortened
         * list. We also provide progress feedback via a signal. */
        if (++cleanupTimer > 4000)
        {
            cleanupTimer = 0;
            TaskList tmpList;
            for (TaskListIterator tli(sortedTasks); *tli != 0; ++tli)
                if (!(*tli)->isSchedulingDone())
                    tmpList.append(*tli);
            sortedTasks = tmpList;
            setProgressBar(leafTasks - sortedTasks.count(), leafTasks);
            setProgressInfo
                (i18n("Scheduling scenario %1 at %1")
                 .arg(getScenarioId(sc)).arg(time2tjp(slot)));
        }

    } while (!done);

    if (error)
        for (TaskListIterator tli(sortedTasks); *tli != 0; ++tli)
            if ((*tli)->isRunaway())
                if ((*tli)->getScheduling() == Task::ASAP)
                    (*tli)->errorMessage
                        (i18n("End of task '%1' does not fit into the "
                              "project time frame. Try using a later project "
                              "end date.")
                         .arg((*tli)->getId()));
                else
                    (*tli)->errorMessage
                        (i18n("Start of task '%1' does not fit into the "
                              "project time frame. Try using an earlier "
                              "project start date.").arg((*tli)->getId()));

    if (!error)
        setProgressBar(100, 100);

    /* Check that the resulting schedule meets all the requirements that the
     * user has specified. */
    setProgressInfo(i18n("Checking schedule of scenario %1")
                    .arg(getScenarioId(sc)));
    if (!checkSchedule(sc))
        error = TRUE;

    return !error;
}

bool
Project::checkSchedule(int sc) const
{
    int errors = 0;
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
    {
        /* Only check top-level tasks, since they recursively check their sub
         * tasks. */
        if ((*tli)->getParent() == 0)
            (*tli)->scheduleOk(sc, errors);
        if (maxErrors > 0 && errors >= maxErrors)
        {
            TJMH.errorMessage
                (i18n("Too many errors in %1 scenario. Giving up.")
                 .arg(getScenarioId(sc)));
            return FALSE;
        }
    }

    return errors == 0;
}

Report*
Project::getReport(uint idx) const
{
    QPtrListIterator<Report> it(reports);
    for (uint i = 0; *it && i < idx; ++it, ++i)
        ;
    return *it;
}

QPtrListIterator<Report>
Project::getReportListIterator() const
{
    return QPtrListIterator<Report>(reports);
}

void
Project::generateReports() const
{
    // Generate reports
    for (QPtrListIterator<Report> ri(reports); *ri != 0; ++ri)
    {
        // We generate all but Qt*Reports. Those are for the GUI version.
        if (strncmp((*ri)->getType(), "Qt", 2) != 0)
        {
            if (DEBUGPS(1))
                qDebug(i18n("Generating report '%1' ...")
                       .arg((*ri)->getFileName()));

            (*ri)->generate();
        }
    }

    generateXMLReport();
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
   bool fatalError;
   if (!pass2(TRUE, fatalError))
       return FALSE;
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
     t->inheritValues();

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

bool Project::generateXMLReport() const
{
    if ( xmlreport )
        return xmlreport->generate();
    else
        return false;
}
