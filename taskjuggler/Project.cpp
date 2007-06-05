/*
 * Project.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Project.h"

#include <stdlib.h>

#include "TjMessageHandler.h"
#include "Scenario.h"
#include "Shift.h"
#include "Account.h"
#include "Task.h"
#include "Resource.h"
#include "VacationInterval.h"
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
#include "CustomAttributeDefinition.h"

DebugController DebugCtrl;

Project::Project() :
    QObject(),
    start(0),
    end(0),
    now(0),
    allowRedefinitions(false),
    weekStartsMonday(true),
    name(),
    version(),
    copyright(),
    customer(),
    timeZone(),
    timeFormat("%Y-%m-%d %H:%M"),
    shortTimeFormat("%H:%M"),
    currency(),
    currencyDigits(3),
    numberFormat("-", "", ",", ".", 1),
    currencyFormat("(", ")", ",", ".", 0),
    priority(500),
    minEffort(0.0),
    resourceLimits(0),
    rate(0.0),
    dailyWorkingHours(8.0),
    yearlyWorkingDays(260.714),
    workingHours(),
    scheduleGranularity(ONEHOUR),
    allowedFlags(),
    projectIDs(),
    currentId(),
    maxErrors(0),
    journal(),
    vacationList(),
    scenarioList(),
    taskList(),
    resourceList(),
    accountList(),
    shiftList(),
    originalTaskList(),
    originalResourceList(),
    originalAccountList(),
    taskAttributes(),
    resourceAttributes(),
    accountAttributes(),
    xmlreport(0),
    reports(),
    interactiveReports(),
    sourceFiles(),
    breakFlag(false)
{
    /* Pick some reasonable initial number since we don't know the
     * project time frame yet. */
    initUtility(20000);

    vacationList.setAutoDelete(true);
    accountAttributes.setAutoDelete(true);
    taskAttributes.setAutoDelete(true);
    resourceAttributes.setAutoDelete(true);
    reports.setAutoDelete(true);

    new Scenario(this, "plan", "Plan", 0);
    scenarioList.createIndex(true);
    scenarioList.createIndex(false);

    setNow(time(0));

    /* Initialize working hours with default values that match the Monday -
     * Friday 9 - 6 (with 1 hour lunch break) pattern used by many western
     * countries. */
    // Sunday
    workingHours[0] = new QPtrList<Interval>();
    workingHours[0]->setAutoDelete(true);

    for (int i = 1; i < 6; ++i)
    {
        workingHours[i] = new QPtrList<Interval>();
        workingHours[i]->setAutoDelete(true);
        workingHours[i]->append(new Interval(9 * ONEHOUR, 12 * ONEHOUR - 1));
        workingHours[i]->append(new Interval(13 * ONEHOUR, 18 * ONEHOUR - 1));
    }

    // Saturday
    workingHours[6] = new QPtrList<Interval>();
    workingHours[6]->setAutoDelete(true);
}

Project::~Project()
{
    taskList.deleteContents();
    resourceList.deleteContents();
    Resource::deleteStaticData();

    accountList.deleteContents();
    shiftList.deleteContents();
    scenarioList.deleteContents();

    delete resourceLimits;

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

bool
Project::setTimeZone(const QString& tz)
{
    if (!setTimezone(tz))
        return false;

    timeZone = tz;
    return true;
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

void
Project::setNow(time_t n)
{
    /* Align 'now' time to timing resolution. If the resolution is
     * changed later, this has to be done again. */
    now = (n / scheduleGranularity) * scheduleGranularity;
}

void
Project::setWorkingHours(int day, const QPtrList<Interval>& l)
{
    if (day < 0 || day > 6)
        qFatal("day out of range");
    delete workingHours[day];

    // Create a deep copy of the interval list.
    workingHours[day] = new QPtrList<Interval>;
    workingHours[day]->setAutoDelete(true);
    for (QPtrListIterator<Interval> pli(l); pli; ++pli)
        workingHours[day]->append(new Interval(**pli));
}

bool
Project::addId(const QString& id, bool changeCurrentId)
{
    if (projectIDs.findIndex(id) != -1)
        return false;
    else
        projectIDs.append(id);

    if (changeCurrentId)
        currentId = id;

    return true;
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
    scenarioList.createIndex(true);
    scenarioList.createIndex(false);
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
        return false;

    taskAttributes.insert(id, cad);
    return true;
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
        return false;

    resourceAttributes.insert(id, cad);
    return true;
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
    if (accountAttributes.find(id))
        return false;

    accountAttributes.insert(id, cad);
    return true;
}

const CustomAttributeDefinition*
Project::getAccountAttribute(const QString& id) const
{
    return accountAttributes[id];
}

bool
Project::isWorkingDay(time_t d) const
{
    return !(workingHours[dayOfWeek(d, false)]->isEmpty() ||
             isVacation(d));
}

bool
Project::isWorkingTime(time_t d) const
{
    if (isVacation(d))
        return false;

    int dow = dayOfWeek(d, false);
    for (QPtrListIterator<Interval> ili(*(workingHours[dow])); *ili != 0; ++ili)
    {
        if ((*ili)->contains(secondsOfDay(d)))
            return true;
    }
    return false;
}

bool
Project::isWorkingTime(const Interval& iv) const
{
    if (isVacation(iv.getStart()))
        return false;

    int dow = dayOfWeek(iv.getStart(), false);
    for (QPtrListIterator<Interval> ili(*(workingHours[dow])); *ili != 0; ++ili)
    {
        if ((*ili)->contains(Interval(secondsOfDay(iv.getStart()),
                                  secondsOfDay(iv.getEnd()))))
            return true;
    }
    return false;
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
    journal.inSort(entry);
}

Journal::Iterator
Project::getJournalIterator() const
{
    return Journal::Iterator(journal);
}

bool
Project::pass2(bool noDepCheck)
{
    int oldErrors = TJMH.getErrors();

    if (taskList.isEmpty())
    {
        TJMH.errorMessage(i18n("The project does not contain any tasks."));
        return false;
    }

    QDict<Task> idHash;

    /* The optimum size for the localtime hash is twice the number of time
     * slots times 2 (because of timeslot and timeslot - 1s). */
    initUtility(4 * ((end - start) / scheduleGranularity));

    // Generate sequence numbers for all lists.
    taskList.createIndex(true);
    resourceList.createIndex(true);
    accountList.createIndex(true);
    shiftList.createIndex(true);

    // Initialize random generator.
    srand((int) start);

    // Create hash to map task IDs to pointers.
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
    {
        idHash.insert((*tli)->getId(), *tli);
    }
    // Create cross links from dependency lists.
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->xRef(idHash);

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
    bool error = false;
    for (ScenarioListIterator sci(scenarioList); *sci; ++sci)
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            if (!(*tli)->preScheduleOk((*sci)->getSequenceNo() - 1))
            {
                error = true;
            }
    if (error)
        return false;

    if (!noDepCheck)
    {
        setProgressInfo(i18n("Searching for dependency loops ..."));
        if (DEBUGPS(1))
            tjDebug("Searching for dependency loops ...");
        // Check all tasks for dependency loops.
        LDIList chkedTaskList;
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            if ((*tli)->loopDetector(chkedTaskList))
                return false;

        setProgressInfo(i18n("Searching for underspecified tasks ..."));
        if (DEBUGPS(1))
            tjDebug("Searching for underspecified tasks ...");
        for (ScenarioListIterator sci(scenarioList); *sci; ++sci)
            for (TaskListIterator tli(taskList); *tli != 0; ++tli)
                if (!(*tli)->checkDetermination((*sci)->getSequenceNo() - 1))
                    error = true;

        if (error)
            return false;
    }

    return TJMH.getErrors() == oldErrors;
}

bool
Project::scheduleScenario(Scenario* sc)
{
    int oldErrors = TJMH.getErrors();

    setProgressInfo(i18n("Scheduling scenario %1...").arg(sc->getId()));

    int scIdx = sc->getSequenceNo() - 1;
    prepareScenario(scIdx);

    if (!schedule(scIdx))
    {
        if (DEBUGPS(2))
            tjDebug(i18n("Scheduling errors in scenario '%1'.")
                   .arg(sc->getId()));
        if (breakFlag)
            return false;
    }
    finishScenario(scIdx);

    for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
    {
        if (!(*rli)->bookingsOk(scIdx))
            break;
    }

    return TJMH.getErrors() == oldErrors;
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
    bool schedulingOk = true;
    for (ScenarioListIterator sci(scenarioList); *sci; ++sci)
        if ((*sci)->getEnabled())
        {
            if (DEBUGPS(1))
                tjDebug(i18n("Scheduling scenario '%1' ...")
                       .arg((*sci)->getId()));

            if (!scheduleScenario(*sci))
                schedulingOk = false;
            if (breakFlag)
                return false;
        }

    completeBuffersAndIndices();

    return schedulingOk;
}

void
Project::overlayScenario(int base, int sc)
{
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        (*tli)->overlayScenario(base, sc);

    for (ScenarioListIterator sli(scenarioList[sc]->getSubListIterator());
         *sli; ++sli)
        overlayScenario(sc, (*sli)->getSequenceNo() - 1);
}

void
Project::prepareScenario(int sc)
{
    for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
        (*rli)->prepareScenario(sc);

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

    if (DEBUGTS(4))
    {
        tjDebug("Allocation probabilities for the resources:");
        for (ResourceListIterator rli(resourceList); *rli != 0; ++rli)
            qDebug("Resource %s: %f%%",
                   (*rli)->getId().latin1(),
                   (*rli)->getAllocationProbability(sc));
        tjDebug("Criticalnesses of the tasks with respect to resource "
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

    /* If the user has not set the minSlackRate to 0 we look for critical
     * pathes. */
    if (getScenario(sc)->getMinSlackRate() > 0.0)
    {
        setProgressInfo(i18n("Computing critical pathes..."));
        /* The critical path detector needs to know the end of the last task.
         * So we have to find this out first. */
        time_t maxEnd = 0;
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        if (maxEnd < (*tli)->getEnd(sc))
            maxEnd = (*tli)->getEnd(sc);

        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
            (*tli)->checkAndMarkCriticalPath
                (sc, getScenario(sc)->getMinSlackRate(), maxEnd);
    }
}

bool
Project::schedule(int sc)
{
    int oldErrors = TJMH.getErrors();

    // The scheduling function only cares about leaf tasks. Container tasks
    // are scheduled automatically when all their childern are scheduled. So
    // we create a task list that only contains leaf tasks.
    TaskList allLeafTasks;
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
        if (!(*tli)->hasSubs())
            allLeafTasks.append(*tli);

    allLeafTasks.setSorting(CoreAttributesList::PrioDown, 0);
    allLeafTasks.setSorting(CoreAttributesList::PathCriticalnessDown, 1);
    allLeafTasks.setSorting(CoreAttributesList::SequenceUp, 2);
    allLeafTasks.sort();

    /* The workItems list contains all tasks that are ready to be scheduled at
     * any given iteration. When a tasks has been scheduled completely, this
     * list needs to be updated again as some tasks may now have become ready
     * to be scheduled. */
    TaskList workItems;
    int sortedTasks = 0;
    for (TaskListIterator tli(allLeafTasks); *tli != 0; ++tli)
    {
        if ((*tli)->isReadyForScheduling())
            workItems.append(*tli);
        if ((*tli)->isSchedulingDone())
            sortedTasks++;
    }

    bool done;
    /* While the scheduling process progresses, the list contains more and
     * more scheduled tasks. We use the cleanupTimer to remove those in
     * certain intervals. As we potentially have already completed tasks in
     * the list when we start, we initialize the timer with a very large
     * number so the first round of cleanup is done right after the first
     * scheduling pass. */
    breakFlag = false;
    bool runAwayFound = false;
    do
    {
        done = true;
        time_t slot = 0;
        int priority = 0;
        Task::SchedulingInfo schedulingInfo = Task::ASAP;

        /* The task list is sorted by priority. The priority decreases towards
         * the end of the list. We iterate through the list and look for a
         * task that can be scheduled. It the determines the time slot that
         * will be scheduled during this run for all subsequent tasks as well.
         */
        for (TaskListIterator tli(workItems); *tli != 0; ++tli)
        {
            if (slot == 0)
            {
                /* No time slot has been set yet. Check if this task can be
                 * scheduled and provides a suggestion. */
                slot = (*tli)->nextSlot(scheduleGranularity);
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
                    runAwayFound = true;
                    slot = 0;
                    continue;
                }
            }
            done = false;
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
            if ((*tli)->schedule(sc, slot, scheduleGranularity))
            {
                workItems.clear();
                int oldSortedTasks = sortedTasks;
                sortedTasks = 0;
                for (TaskListIterator tli(allLeafTasks); *tli != 0; ++tli)
                {
                    if ((*tli)->isReadyForScheduling())
                        workItems.append(*tli);
                    if ((*tli)->isSchedulingDone())
                        sortedTasks++;
                }
                // Update the progress bar after every 10th completed tasks.
                if (oldSortedTasks / 10 != sortedTasks / 10)
                {
                    setProgressBar(sortedTasks - allLeafTasks.count(),
                                   sortedTasks);
                    setProgressInfo
                        (i18n("Scheduling scenario %1 at %2")
                         .arg(getScenarioId(sc)).arg(time2tjp(slot)));
                }
            }
        }
    } while (!done && !breakFlag);

    if (breakFlag)
    {
        setProgressInfo("");
        setProgressBar(0, 0);
        TJMH.errorMessage(i18n("Scheduling aborted on user request"));
        return false;
    }

    if (runAwayFound)
        for (TaskListIterator tli(taskList); *tli != 0; ++tli)
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

    if (TJMH.getErrors() == oldErrors)
        setProgressBar(100, 100);

    /* Check that the resulting schedule meets all the requirements that the
     * user has specified. */
    setProgressInfo(i18n("Checking schedule of scenario %1")
                    .arg(getScenarioId(sc)));
    checkSchedule(sc);

    return TJMH.getErrors() == oldErrors;
}

void
Project::breakScheduling()
{
    breakFlag = true;
}

bool
Project::checkSchedule(int sc) const
{
    int oldErrors = TJMH.getErrors();
    for (TaskListIterator tli(taskList); *tli != 0; ++tli)
    {
        /* Only check top-level tasks, since they recursively check their sub
         * tasks. */
        if ((*tli)->getParent() == 0)
            (*tli)->scheduleOk(sc);
        if (maxErrors > 0 && TJMH.getErrors() >= maxErrors)
        {
            TJMH.errorMessage
                (i18n("Too many errors in %1 scenario. Giving up.")
                 .arg(getScenarioId(sc)));
            return false;
        }
    }

    return TJMH.getErrors() == oldErrors;
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

bool
Project::generateReports() const
{
    // Generate reports
    int errors = 0;
    for (QPtrListIterator<Report> ri(reports); *ri != 0; ++ri)
    {
        // We generate all but Qt*Reports. Those are for the GUI version.
        if (strncmp((*ri)->getType(), "Qt", 2) != 0)
        {
            if (DEBUGPS(1))
                tjDebug(i18n("Generating report '%1' ...")
                       .arg((*ri)->getFileName()));

            if (!(*ri)->generate())
                errors++;
        }
    }

    generateXMLReport();

    return errors == 0;
}

bool Project::generateXMLReport() const
{
    if ( xmlreport )
        return xmlreport->generate();
    else
        return false;
}
