/*
 * Task.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdlib.h>
#include <math.h>

#include "Task.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "debug.h"
#include "Resource.h"
#include "Account.h"
#include "Project.h"
#include "ResourceTreeIterator.h"
#include "Allocation.h"
#include "Booking.h"
#include "ReportXML.h"
#include "Scenario.h"
#include "CustomAttributeDefinition.h"
#include "UsageLimits.h"
#include "OptimizerRun.h"
#include "Journal.h"

Task::Task(Project* proj, const QString& id_, const QString& n, Task* p,
           const QString& df, int dl)
    : CoreAttributes(proj, id_, n, p, df, dl)
{
    allocations.setAutoDelete(TRUE);
    shifts.setAutoDelete(TRUE);
    depends.setAutoDelete(TRUE);
    precedes.setAutoDelete(TRUE);

    proj->addTask(this);

    scheduling = ASAP;
    milestone = FALSE;
    account = 0;
    lastSlot = 0;
    doneEffort = 0.0;
    doneDuration = 0.0;
    doneLength = 0.0;
    schedulingDone = FALSE;
    responsible = 0;

    scenarios = new TaskScenario[proj->getMaxScenarios()];
    for (int i = 0; i < proj->getMaxScenarios(); i++)
    {
        scenarios[i].task = this;
        scenarios[i].index = i;
    }

    scenarios[0].startBuffer = 0.0;
    scenarios[0].endBuffer = 0.0;
    scenarios[0].startCredit = 0.0;
    scenarios[0].endCredit = 0.0;

    start = end = 0;
    duration = length = effort = 0.0;

    for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
    {
        scenarios[sc].minStart = scenarios[sc].minEnd = 0;
        scenarios[sc].maxStart = scenarios[sc].maxEnd = 0;
    }
}

Task::~Task()
{
    project->deleteTask(this);
    delete [] scenarios;
}

void
Task::inheritValues()
{
    Task* p = (Task*) parent;
    if (p)
    {
        // Inherit flags from parent task.
        for (QStringList::Iterator it = p->flags.begin();
             it != p->flags.end(); ++it)
            addFlag(*it);

        // Set attributes that are inherited from parent task.
        projectId = p->projectId;
        priority = p->priority;
        responsible = p->responsible;
        account = p->account;
        scheduling = p->scheduling;

        for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
        {
            scenarios[sc].minStart = p->scenarios[sc].minStart;
            scenarios[sc].maxStart = p->scenarios[sc].maxEnd;
            scenarios[sc].minEnd = p->scenarios[sc].minStart;
            scenarios[sc].maxEnd = p->scenarios[sc].maxEnd;
        }
        // Inherit depends from parent. Relative IDs need to get another '!'.
        for (QPtrListIterator<TaskDependency> tdi(p->depends); tdi; ++tdi)
        {
            QString id = (*tdi)->getTaskRefId();
            if (id[0] == '!')
                id = '!' + id;
            TaskDependency* td = new TaskDependency(id,
                                                    project->getMaxScenarios());
            for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
            {
                td->setGapDuration(sc, (*tdi)->getGapDuration(sc));
                td->setGapLength(sc, (*tdi)->getGapLength(sc));
            }
            depends.append(td);
        }

        // Inherit precedes from parent. Relative IDs need to get another '!'.
        for (QPtrListIterator<TaskDependency> tdi(p->precedes); *tdi; ++tdi)
        {
            QString id = (*tdi)->getTaskRefId();
            if (id[0] == '!')
                id = '!' + id;
            TaskDependency* td = new TaskDependency(id,
                                                    project->getMaxScenarios());
            for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
            {
                td->setGapDuration(sc, (*tdi)->getGapDuration(sc));
                td->setGapLength(sc, (*tdi)->getGapLength(sc));
            }
            depends.append(td);
        }

        // Inherit allocations from parent.
        for (QPtrListIterator<Allocation> ali(p->allocations); *ali; ++ali)
            allocations.append(new Allocation(**ali));

        // Inherit inheritable custom attributes
        inheritCustomAttributes(project->getTaskAttributeDict());
    }
    else
    {
        // Set attributes that are inherited from global attributes.
        projectId = project->getCurrentId();
        priority = project->getPriority();
        for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
        {
            scenarios[sc].minStart = scenarios[sc].minEnd = 0;
            scenarios[sc].maxStart = scenarios[sc].maxEnd = 0;
        }
    }
}

void
Task::addJournalEntry(JournalEntry* entry)
{
    journal.append(entry);
}

JournalIterator
Task::getJournalIterator() const
{
    return JournalIterator(journal);
}

TaskDependency*
Task::addDepends(const QString& rid)
{
    TaskDependency* td = new TaskDependency(rid, project->getMaxScenarios());
    depends.append(td);
    return td;
}

TaskDependency*
Task::addPrecedes(const QString& rid)
{
    TaskDependency* td = new TaskDependency(rid, project->getMaxScenarios());
    precedes.append(td);
    return td;
}

bool
Task::addShift(const Interval& i, Shift* s)
{
    return shifts.insert(new ShiftSelection(i, s));
}

void
Task::errorMessage(const char* msg, ...) const
{
    va_list ap;
    va_start(ap, msg);
    char buf[2048];
    vsnprintf(buf, sizeof(buf), msg, ap);
    va_end(ap);

    TJMH.errorMessage(buf, definitionFile, definitionLine);
}

void
Task::schedule(int sc, time_t& date, time_t slotDuration)
{
    // Has the task been scheduled already or is it a container?
    if (schedulingDone || !sub->isEmpty())
        return;

    if (DEBUGTS(15))
        qDebug("Trying to schedule %s at %s",
               id.latin1(), time2tjp(date).latin1());

    if (scheduling == Task::ASAP)
    {
        if (start == 0 ||
            (effort == 0.0 && length == 0.0 && duration == 0.0 && end == 0))
            return;

        if (lastSlot == 0)
        {
            lastSlot = start - 1;
            tentativeEnd = date + slotDuration - 1;
            if (DEBUGTS(5))
                qDebug("Scheduling of %s starts at %s (%s)",
                       id.latin1(), time2tjp(start).latin1(),
                       time2tjp(date).latin1());
        }
        /* Do not schedule anything if the time slot is not directly
         * following the time slot that was previously scheduled. */
        if (!((date - slotDuration <= lastSlot) && (lastSlot < date)))
            return;
        lastSlot = date + slotDuration - 1;
    }
    else
    {
        if (end == 0 ||
            (effort == 0.0 && length == 0.0 && duration == 0.0 && start == 0))
            return;

        if (lastSlot == 0)
        {
            lastSlot = end + 1;
            tentativeStart = date;
            if (DEBUGTS(5))
                qDebug("Scheduling of ALAP task %s starts at %s (%s)",
                       id.latin1(), time2tjp(lastSlot).latin1(),
                       time2tjp(date).latin1());
        }
        /* Do not schedule anything if the current time slot is not
         * directly preceding the previously scheduled time slot. */
        if (!((date + slotDuration <= lastSlot) &&
            (lastSlot < date + 2 * slotDuration)))
            return;
        lastSlot = date;
    }

    if (DEBUGTS(10))
        qDebug("Scheduling %s at %s",
               id.latin1(), time2tjp(date).latin1());

    if ((duration > 0.0) || (length > 0.0))
    {
        /* Length specifies the number of working days (as daily load)
         * and duration specifies the number of calender days. */
        if (!allocations.isEmpty())
            bookResources(sc, date, slotDuration);

        doneDuration += ((double) slotDuration) / ONEDAY;
        if (project->isWorkingTime(Interval(date, date + slotDuration - 1)))
            doneLength += project->convertToDailyLoad(slotDuration);

        if (DEBUGTS(10))
            qDebug("Length: %f/%f   Duration: %f/%f",
                   doneLength, length,
                   doneDuration, duration);
        // Check whether we are done with this task.
        /* The accumulated done* values contain rounding errors. This prevents
         * exact float comparisons. To avoid rounding problems we compare the
         * rounded values of the done* values multiplied by 2048. This should
         * result in worst case errors of smaller than a minute. The value
         * 2048 was chosen in the hope that a compiler is clever enough to
         * avoid a costly multiplication if possible. */
        if ((length > 0.0 &&
             qRound(doneLength * 2048) >= qRound(length * 2048)) ||
            (duration > 0.0 &&
             qRound(doneDuration * 2048) >= qRound(duration * 2048)))
        {
            if (scheduling == ASAP)
            {
                end = date + slotDuration - 1;
                propagateEnd(sc);
            }
            else
            {
                start = date;
                propagateStart(sc);
            }
            schedulingDone = TRUE;
            if (DEBUGTS(4))
                qDebug("Scheduling of task %s completed", id.latin1());
            return;
        }
    }
    else if (effort > 0.0)
    {
        /* The effort of the task has been specified. We have to look
         * how much the resources can contribute over the following
         * workings days until we have reached the specified
         * effort. */
        bookResources(sc, date, slotDuration);
        // Check whether we are done with this task.
        if (qRound(doneEffort * 2048) >= qRound(effort * 2048))
        {
            if (scheduling == ASAP)
            {
                end = tentativeEnd;
                propagateEnd(sc);
            }
            else
            {
                start = tentativeStart;
                propagateStart(sc);
            }
            schedulingDone = TRUE;
            if (DEBUGTS(4))
                qDebug("Scheduling of task %s completed", id.latin1());
            return;
        }
    }
    else if (milestone)
    {
        // Task is a milestone.
        if (scheduling == ASAP)
        {
            end = start - 1;
            propagateEnd(sc);
        }
        else
        {
            start = end + 1;
            propagateStart(sc);
        }
        return;
    }
    else if (start != 0 && end != 0)
    {
        // Task with start and end date but no duration criteria.
        if (!allocations.isEmpty() && !project->isVacation(date))
            bookResources(sc, date, slotDuration);

        if ((scheduling == ASAP && (date + slotDuration) >= end) ||
            (scheduling == ALAP && date <= start))
        {
            schedulingDone = TRUE;
            if (DEBUGTS(4))
                qDebug("Scheduling of task %s completed", id.latin1());
            return;
        }
    }

    return;
}

bool
Task::scheduleContainer(int sc, bool safeMode)
{
    if (schedulingDone)
        return TRUE;

    time_t nstart = 0;
    time_t nend = 0;

    TaskListIterator tli(*sub);
    // Check that this is really a container task
    if (*tli != 0)
    {
        if ((*tli)->start == 0 || (*tli)->end == 0)
            return TRUE;
        nstart = (*tli)->start;
        nend = (*tli)->end;
    }
    else
        return TRUE;

    for ( ++tli; *tli != 0; ++tli)
    {
        /* Make sure that all sub tasks have been scheduled. If not we
         * can't yet schedule this task. */
        if ((*tli)->start == 0 || (*tli)->end == 0)
            return TRUE;

        if ((*tli)->start < nstart)
            nstart = (*tli)->start;
        if ((*tli)->end > nend)
            nend = (*tli)->end;
    }

    if (start == 0 || (!depends.isEmpty() && start < nstart))
    {
        start = nstart;
        propagateStart(sc, safeMode);
    }

    if (end == 0 || (!precedes.isEmpty() && nend < end))
    {
        end = nend;
        propagateEnd(sc, safeMode);
    }

    if (DEBUGTS(4))
        qDebug("Scheduling of task %s completed", id.latin1());
    schedulingDone = TRUE;

    return FALSE;
}

void
Task::propagateStart(int sc, bool notUpwards)
{
    if (start == 0)
        return;

    if (DEBUGTS(11))
        qDebug("PS1: Setting start of %s to %s",
               id.latin1(), time2tjp(start).latin1());

    /* If one end of a milestone is fixed, then the other end can be set as
     * well. */
    if (milestone)
    {
        schedulingDone = TRUE;
        if (end == 0)
        {
            end = start - 1;
            propagateEnd(sc, notUpwards);
        }
    }

    /* Set start date to all previous that have no start date yet, but are
     * ALAP task or have no duration. */
    for (TaskListIterator tli(previous); *tli != 0; ++tli)
        if ((*tli)->end == 0 && (*tli)->latestEnd(sc) != 0 &&
            !(*tli)->schedulingDone &&
            ((*tli)->scheduling == ALAP ||
             ((*tli)->effort == 0.0 && (*tli)->length == 0.0 &&
              (*tli)->duration == 0.0 && !(*tli)->milestone)))
        {
            (*tli)->end = (*tli)->latestEnd(sc);
            if (DEBUGTS(11))
                qDebug("PS2: Setting end of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->end).latin1());
            /* Recursively propagate the end date */
            (*tli)->propagateEnd(sc, notUpwards);
        }

    /* Propagate start time to sub tasks which have only an implicit
     * dependancy on the parent task. Do not touch container tasks. */
    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
    {
        if (!(*tli)->hasStartDependency() && !(*tli)->schedulingDone)
        {
            (*tli)->start = start;
            if (DEBUGTS(11))
                qDebug("PS3: Setting start of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->start).latin1());
            /* Recursively propagate the start date */
            (*tli)->propagateStart(sc, TRUE);
        }
    }

    if (notUpwards && parent)
    {
        if (DEBUGTS(11))
            qDebug("Scheduling parent of %s", id.latin1());
        getParent()->scheduleContainer(sc, TRUE);
    }
}

void
Task::propagateEnd(int sc, bool notUpwards)
{
    if (end == 0)
        return;

    if (DEBUGTS(11))
        qDebug("PE1: Setting end of %s to %s",
               id.latin1(), time2tjp(end).latin1());

    /* If one end of a milestone is fixed, then the other end can be set as
     * well. */
    if (milestone)
    {
        if (DEBUGTS(4))
            qDebug("Scheduling of task %s completed", id.latin1());
        schedulingDone = TRUE;
        if (start == 0)
        {
            start = end + 1;
            propagateStart(sc, notUpwards);
        }
    }

    /* Set start date to all followers that have no start date yet, but are
     * ASAP task or have no duration. */
    for (TaskListIterator tli(followers); *tli != 0; ++tli)
        if ((*tli)->start == 0 && (*tli)->earliestStart(sc) != 0 &&
            !(*tli)->schedulingDone &&
            ((*tli)->scheduling == ASAP ||
             ((*tli)->effort == 0.0 && (*tli)->length == 0.0 &&
              (*tli)->duration == 0.0 && !(*tli)->milestone)))
        {
            (*tli)->start = (*tli)->earliestStart(sc);
            if (DEBUGTS(11))
                qDebug("PE2: Setting start of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->start).latin1());
            /* Recursively propagate the start date */
            (*tli)->propagateStart(sc, notUpwards);
        }
    /* Propagate end time to sub tasks which have only an implicit
     * dependancy on the parent task. Do not touch container tasks. */
    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        if (!(*tli)->hasEndDependency() && !(*tli)->schedulingDone)
        {
            (*tli)->end = end;
            if (DEBUGTS(11))
                qDebug("PE3: Setting end of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->end).latin1());
            /* Recursively propagate the end date */
            (*tli)->propagateEnd(sc, TRUE);
        }

    if (notUpwards && parent)
    {
        if (DEBUGTS(11))
            qDebug("Scheduling parent of %s", id.latin1());
        getParent()->scheduleContainer(sc, TRUE);
    }
}

void
Task::propagateInitialValues(int sc)
{
    if (start != 0)
        propagateStart(sc, TRUE);
    if (end != 0)
        propagateEnd(sc, TRUE);

    // Check if the some data of sub tasks can already be propagated.
    if (!sub->isEmpty())
        scheduleContainer(sc, TRUE);
}

void
Task::setRunaway()
{
    schedulingDone = TRUE;
    runAway = TRUE;
}

bool
Task::isRunaway() const
{
    /* If a container task has runaway sub tasts, it is very likely that they
     * are the culprits. So we don't report such a container task as runaway.
     */
    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        if ((*tli)->isRunaway())
            return FALSE;

    return runAway;
}

void
Task::bookResources(int sc, time_t date, time_t slotDuration)
{
    /* If the time slot overlaps with a specified shift interval, the
     * time slot must also be within the specified working hours of that
     * shift interval. */
    if (!shifts.isOnShift(Interval(date, date + slotDuration - 1)))
    {
        if (DEBUGRS(15))
            qDebug("Task %s is not active at %s", id.latin1(),
                   time2tjp(date).latin1());
        return;
    }

    /* If any of the resources is marked as being mandatory, we have to check
     * if this resource is available. In case it's not available we do not
     * allocate any of the other resources for the time slot. */
    bool allMandatoriesAvailables = TRUE;
    for (QPtrListIterator<Allocation> ali(allocations); *ali != 0; ++ali)
        if ((*ali)->isMandatory())
        {
            if (!(*ali)->isOnShift(Interval(date, date + slotDuration - 1)))
            {
                allMandatoriesAvailables = FALSE;
                break;
            }
            if ((*ali)->isPersistent() && (*ali)->getLockedResource())
            {
                int availability;
                if ((availability = (*ali)->getLockedResource()->
                     isAvailable(date)) > 0)
                {
                    allMandatoriesAvailables = FALSE;
                    if (availability >= 4 && !(*ali)->getConflictStart())
                        (*ali)->setConflictStart(date);
                    break;
                }
            }
            else
            {
                /* For a mandatory allocation with alternatives at least one
                 * of the resources or resource groups must be available. */
                bool found = FALSE;
                int maxAvailability = 0;
                QPtrList<Resource> candidates = (*ali)->getCandidates();
                for (QPtrListIterator<Resource> rli(candidates);
                     *rli && !found; ++rli)
                {
                    /* If a resource group is marked mandatory, all members
                     * of the group must be available. */
                    int availability;
                    bool allAvailable = TRUE;
                    for (ResourceTreeIterator rti(*rli); *rti != 0; ++rti)
                        if ((availability =
                             (*rti)->isAvailable(date)) > 0)
                        {
                            allAvailable = FALSE;
                            if (availability >= maxAvailability)
                                maxAvailability = availability;
                        }
                    if (allAvailable)
                        found = TRUE;
                }
                if (!found)
                {
                    if (maxAvailability >= 4 && !(*ali)->getConflictStart())
                        (*ali)->setConflictStart(date);
                    allMandatoriesAvailables = FALSE;
                    break;
                }
            }
        }

    for (QPtrListIterator<Allocation> ali(allocations);
         *ali != 0 && allMandatoriesAvailables &&
         (effort == 0.0 || doneEffort < effort); ++ali)
    {
        /* If a shift has been defined for a resource for this task, there
         * must be a shift interval defined for this day and the time must
         * be within the working hours of that shift. */
        if (!(*ali)->isOnShift(Interval(date, date + slotDuration - 1)))
        {
            if (DEBUGRS(15))
                qDebug("Allocation not on shift at %s",
                       time2tjp(date).latin1());
            continue;
        }

        /* Now check the limits set for this allocation. */
        const UsageLimits* limits = (*ali)->getLimits();
        if (limits)
        {
            QPtrList<Resource> resources = (*ali)->getCandidates();
            if (limits->getDailyMax() > 0)
            {
                uint timeSlots = 0;
                for (QPtrListIterator<Resource> rli(resources); *rli; ++rli)
                    timeSlots += (*rli)->getCurrentDaySlots(date, this);
                if (timeSlots > limits->getDailyMax())
                {
                    if (DEBUGRS(6))
                        qDebug("  Allocation overloaded today for task %s",
                               id.latin1());
                    continue;
                }
            }
            if (limits->getWeeklyMax() > 0)
            {
                uint timeSlots = 0;
                for (QPtrListIterator<Resource> rli(resources); *rli; ++rli)
                    timeSlots += (*rli)->getCurrentWeekSlots(date, this);
                if (timeSlots > limits->getWeeklyMax())
                {
                    if (DEBUGRS(6))
                        qDebug("  Allocation overloaded this week for task %s",
                               id.latin1());
                    continue;
                }
            }
            if (limits->getMonthlyMax() > 0)
            {
                uint timeSlots = 0;
                for (QPtrListIterator<Resource> rli(resources); *rli; ++rli)
                    timeSlots += (*rli)->getCurrentMonthSlots(date, this);
                if (timeSlots > limits->getMonthlyMax())
                {
                    if (DEBUGRS(6))
                        qDebug("  Allocation overloaded this month for task %s",
                               id.latin1());
                    continue;
                }
            }
        }

        /* If the allocation has be marked persistent and a resource
         * has already been picked, try to book this resource again. If the
         * resource is not available there will be no booking for this
         * time slot. */
        int maxAvailability = 0;
        if ((*ali)->isPersistent() && (*ali)->getLockedResource())
        {
            if (!bookResource((*ali)->getLockedResource(), date, slotDuration,
                              maxAvailability))
            {
                if (maxAvailability >= 4 && !(*ali)->getConflictStart())
                    (*ali)->setConflictStart(date);
            }
            else if ((*ali)->getConflictStart())
            {
                if (DEBUGRS(2))
                    qDebug("Resource %s is not available for task '%s' "
                           "from %s to %s",
                           (*ali)->getLockedResource()->getId().latin1(),
                           id.latin1(),
                           time2ISO((*ali)->getConflictStart()).latin1(),
                           time2ISO(date).latin1());
                (*ali)->setConflictStart(0);
            }
        }
        else
        {
            QPtrList<Resource> cl = createCandidateList(sc, date, *ali);

            bool found = FALSE;
            for (QPtrListIterator<Resource> rli(cl); *rli != 0; ++rli)
                if (bookResource((*rli), date, slotDuration,
                                 maxAvailability))
                {
                    (*ali)->setLockedResource(*rli);
                    found = TRUE;
                    break;
                }
            if (!found && maxAvailability >= 4 && !(*ali)->getConflictStart())
                (*ali)->setConflictStart(date);
            else if (found && (*ali)->getConflictStart())
            {
                if (DEBUGRS(2))
                {
                    QString candidates;
                    bool first = TRUE;
                    for (QPtrListIterator<Resource> rli(cl); *rli != 0; ++rli)
                    {
                        if (first)
                            first = FALSE;
                        else
                            candidates += ", ";
                        candidates += (*rli)->getId();
                    }
                    qDebug("No resource of the allocation (%s) is available "
                           "for task '%s' from %s to %s",
                           candidates.latin1(),
                           id.latin1(),
                           time2ISO((*ali)->getConflictStart()).latin1(),
                           time2ISO(date).latin1());
                }
                (*ali)->setConflictStart(0);
            }
        }
    }
}

bool
Task::bookResource(Resource* r, time_t date, time_t slotDuration,
                   int& maxAvailability)
{
    bool booked = FALSE;
    double intervalLoad = project->convertToDailyLoad(slotDuration);

    for (ResourceTreeIterator rti(r); *rti != 0; ++rti)
    {
        int availability;
        if ((availability =
             (*rti)->isAvailable(date)) == 0)
        {
            (*rti)->book(new Booking(Interval(date, date + slotDuration - 1),
                                     this));
            addBookedResource(*rti);

            /* Move the start date to make sure that there is
             * some work going on at the start date. */
            if (!workStarted)
            {
                if (scheduling == ASAP)
                    start = date;
                else if (scheduling == ALAP)
                    end = date + slotDuration - 1;
                else
                    qFatal("Unknown scheduling mode");
                workStarted = TRUE;
            }

            tentativeStart = date;
            tentativeEnd = date + slotDuration - 1;
            doneEffort += intervalLoad * (*rti)->getEfficiency();

            if (DEBUGTS(6))
                qDebug(" Booked resource %s (Effort: %f)",
                       (*rti)->getId().latin1(), doneEffort);
            booked = TRUE;
        }
        else if (availability > maxAvailability)
            maxAvailability = availability;
    }
    return booked;
}

QPtrList<Resource>
Task::createCandidateList(int sc, time_t date, Allocation* a)
{
    /* This function generates a list of resources that could be allocated to
     * the task. The order of the list is determined by the specified
     * selection function of the alternatives list. From this list, the
     * first available resource is picked later on. */
    QPtrList<Resource> candidates = a->getCandidates();
    QPtrList<Resource> cl;

    /* We try to minimize resource changes for consecutives time slots. So
     * the resource used for the previous time slot is put to the 1st position
     * of the list. */
    if (a->getLockedResource())
    {
        cl.append(a->getLockedResource());
        candidates.remove(a->getLockedResource());
        /* When an allocation is booked the resource is saved as locked
         * resource. */
        a->setLockedResource(0);
    }
    switch (a->getSelectionMode())
    {
        case Allocation::order:
            if (DEBUGTS(25))
                qDebug("order");
            while (candidates.getFirst())
            {
                cl.append(candidates.getFirst());
                candidates.remove(candidates.getFirst());
            }
            break;
        case Allocation::minAllocationProbability:
        {
            if (DEBUGTS(25))
                qDebug("minAllocationProbability");
            /* This is another heuristic to optimize scheduling results. The
             * idea is to pick the resource that is most likely to be used
             * least during this project (because of the specified
             * allocations) and try to use it first. Unfortunately this
             * algorithm can make things worse in certain plan setups. */
            while (!candidates.isEmpty())
            {
                /* Find canidate with smallest allocationProbability and
                 * append it to the candidate list. */
                double minProbability = 0;
                Resource* minProbResource = 0;
                for (QPtrListIterator<Resource> rli(candidates);
                     *rli != 0; ++rli)
                {
                    double probability = (*rli)->getAllocationProbability(sc);
                    if (minProbability == 0 || probability < minProbability)
                    {
                        minProbability = probability;
                        minProbResource = *rli;
                    }
                }
                cl.append(minProbResource);
                candidates.remove(minProbResource);
            }
            break;
        }
        case Allocation::minLoaded:
        {
            if (DEBUGTS(25))
                qDebug("minLoad");
            while (!candidates.isEmpty())
            {
                double minLoad = 0;
                Resource* minLoaded = 0;
                for (QPtrListIterator<Resource> rli(candidates);
                     *rli != 0; ++rli)
                {
                    /* We calculate the load as a relative value to the daily
                     * max load. This way part time people will reach their
                     * max as slowly as the full timers. */
                    double load =
                        (*rli)->getCurrentLoad(Interval(project->getStart(),
                                                        date), 0) /
                        (((*rli)->getLimits() &&
                          (*rli)->getLimits()->getDailyMax() > 0) ?
                         project->convertToDailyLoad
                         ((*rli)->getLimits()->getDailyMax() *
                          project->getScheduleGranularity()) : 1.0);

                    if (minLoaded == 0 || load < minLoad)
                    {
                        minLoad = load;
                        minLoaded = *rli;
                    }
                }
                cl.append(minLoaded);
                candidates.remove(minLoaded);
            }
            break;
        }
        case Allocation::maxLoaded:
        {
            if (DEBUGTS(25))
                qDebug("maxLoad");
            while (!candidates.isEmpty())
            {
                double maxLoad = 0;
                Resource* maxLoaded = 0;
                for (QPtrListIterator<Resource> rli(candidates);
                     *rli != 0; ++rli)
                {
                    /* We calculate the load as a relative value to the daily
                     * max load. This way part time people will reach their
                     * max as fast as the full timers. */
                    double load =
                        (*rli)->getCurrentLoad(Interval(project->getStart(),
                                                        date), 0) /
                        (((*rli)->getLimits() &&
                          (*rli)->getLimits()->getDailyMax() > 0) ?
                         project->convertToDailyLoad
                         ((*rli)->getLimits()->getDailyMax() *
                          project->getScheduleGranularity()) : 1.0);

                    if (maxLoaded == 0 || load > maxLoad)
                    {
                        maxLoad = load;
                        maxLoaded = *rli;
                    }
                }
                cl.append(maxLoaded);
                candidates.remove(maxLoaded);
            }
            break;
        }
        case Allocation::random:
        {
            if (DEBUGTS(25))
                qDebug("random");
            while (candidates.getFirst())
            {
                cl.append(candidates.at(rand() % candidates.count()));
                candidates.remove(candidates.getFirst());
            }
            break;
        }
        default:
            qFatal("Illegal selection mode %d", a->getSelectionMode());
    }

    return cl;
}

bool
Task::isCompleted(int sc, time_t date) const
{
    if (scenarios[sc].reportedCompletion >= 0.0)
    {
        // some completion degree has been specified.
        if (scenarios[sc].effort > 0.0)
        {
            if (date < scenarios[sc].start)
                return FALSE;
            return qRound((scenarios[sc].effort *
                           (scenarios[sc].reportedCompletion / 100.0)) * 1000)
                >= qRound(getLoad(sc, Interval(scenarios[sc].start, date), 0)
                         * 1000);
        }
        else
            return ((scenarios[sc].reportedCompletion / 100.0) *
                    (scenarios[sc].end - scenarios[sc].start)
                    + scenarios[sc].start) > date;
    }


    return (project->getNow() > date);
}

bool
Task::isBuffer(int sc, const Interval& iv) const
{
    return iv.overlaps(Interval(scenarios[sc].start,
                                scenarios[sc].startBufferEnd)) ||
        iv.overlaps(Interval(scenarios[sc].endBufferStart,
                             scenarios[sc].end));
}

time_t
Task::earliestStart(int sc) const
{
    time_t date = 0;
    for (QPtrListIterator<TaskDependency> tdi(depends); *tdi != 0; ++tdi)
    {
        // All tasks this task depends on must have an end date set.
        if ((*tdi)->getTaskRef()->end == 0)
            return 0;

        /* Add the gapDuration and/or gapLength to the end of the dependent
         * task. */
        time_t potentialDate = (*tdi)->getTaskRef()->end + 1;
        time_t dateAfterLengthGap;
        long gapLength = (*tdi)->getGapLength(sc);
        for (dateAfterLengthGap = potentialDate;
             gapLength > 0 && dateAfterLengthGap < project->getEnd();
             dateAfterLengthGap += project->getScheduleGranularity())
            if (project->isWorkingTime(dateAfterLengthGap))
                gapLength -= project->getScheduleGranularity();
        if (dateAfterLengthGap > potentialDate + (*tdi)->getGapDuration(sc))
            potentialDate = dateAfterLengthGap;
        else
            potentialDate += (*tdi)->getGapDuration(sc);
        // Set 'date' to the latest end date plus gaps of all preceding tasks.
        if (potentialDate > date)
            date = potentialDate;
    }
    /* If any of the parent tasks has an explicit start date, the task must
     * start at or after this date. */
    for (Task* t = getParent(); t; t = t->getParent())
        if (t->start > date)
            return t->start;

    return date;
}

time_t
Task::latestEnd(int sc) const
{
    time_t date = 0;
    for (QPtrListIterator<TaskDependency> tdi(precedes); *tdi; ++tdi)
    {
        // All tasks this task precedes must have a start date set.
        if ((*tdi)->getTaskRef()->start == 0)
            return 0;

        /* Subtract the gapDuration and/or gapLength from the start of the
         * following task. */
        time_t potentialDate = (*tdi)->getTaskRef()->start - 1;
        time_t dateBeforeLengthGap;
        long gapLength = (*tdi)->getGapLength(sc);
        for (dateBeforeLengthGap = potentialDate;
             gapLength > 0 && dateBeforeLengthGap >= project->getStart();
             dateBeforeLengthGap -= project->getScheduleGranularity())
            if (project->isWorkingTime(dateBeforeLengthGap))
                gapLength -= project->getScheduleGranularity();
        if (dateBeforeLengthGap < potentialDate - (*tdi)->getGapDuration(sc))
            potentialDate = dateBeforeLengthGap;
        else
            potentialDate -= (*tdi)->getGapDuration(sc);

        /* Set 'date' to the earliest end date minus gaps of all following
         * tasks. */
        if (date == 0 || potentialDate < date)
            date = potentialDate;
    }
    /* If any of the parent tasks has an explicit end date, the task must
     * end at or before this date. */
    for (Task* t = getParent(); t; t = t->getParent())
        if (t->end != 0 && t->end < date)
            return t->end;

    return date;
}

double
Task::getCalcEffort(int sc) const
{
    if (milestone)
        return 0.0;

    return getLoad(sc, Interval(scenarios[sc].start, scenarios[sc].end));
}

double
Task::getCalcDuration(int sc) const
{
    if (milestone)
        return 0.0;

    time_t delta = scenarios[sc].end - scenarios[sc].start;
    if (delta < ONEDAY)
        return (project->convertToDailyLoad(delta));
    else
        return (double) delta / ONEDAY;
}

double
Task::getLoad(int sc, const Interval& period, const Resource* resource) const
{
    if (milestone)
        return 0.0;

    double load = 0.0;

    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        load += (*tli)->getLoad(sc, period, resource);

    if (resource)
        load += resource->getLoad(sc, period, AllAccounts, this);
    else
        for (ResourceListIterator rli(scenarios[sc].bookedResources);
             *rli != 0; ++rli)
            load += (*rli)->getLoad(sc, period, AllAccounts, this);

    return load;
}

double
Task::getCredits(int sc, const Interval& period, AccountType acctType,
                 const Resource* resource, bool recursive) const
{
    double credits = 0.0;

    if (recursive && !sub->isEmpty())
    {
        for (TaskListIterator tli(*sub); *tli != 0; ++tli)
            credits += (*tli)->getCredits(sc, period, acctType, resource,
                                          recursive);
    }

    if (acctType != AllAccounts &&
        (account == 0 || acctType != account->getAcctType()))
        return credits;

    if (resource)
        credits += resource->getCredits(sc, period, acctType, this);
    else
        for (ResourceListIterator rli(scenarios[sc].bookedResources);
             *rli != 0; ++rli)
            credits += (*rli)->getCredits(sc, period, acctType, this);

    if (period.contains(scenarios[sc].start))
        credits += scenarios[sc].startCredit;
    if (period.contains(scenarios[sc].end + (milestone ? 1 : 0)))
        credits += scenarios[sc].endCredit;

    return credits;
}

bool
Task::xRef(QDict<Task>& hash)
{
    bool error = FALSE;

    if (DEBUGPF(5))
        qDebug("Creating cross references for task %s ...", id.latin1());

    for (QPtrListIterator<TaskDependency> tdi(depends); *tdi; ++tdi)
    {
        QString absId = resolveId((*tdi)->getTaskRefId());
        Task* t;
        if ((t = hash.find(absId)) == 0)
        {
            errorMessage(i18n("Unknown dependency '%1'").arg(absId));
            error = TRUE;
        }
        else
        {
            for (QPtrListIterator<TaskDependency> tdi2(depends); *tdi2; ++tdi2)
                if ((*tdi2)->getTaskRef() == t)
                {
                    errorMessage(i18n("No need to specify dependency %1 "
                                      "multiple times.").arg(absId));
                    error = TRUE;
                    break;
                }
            if (error)
            {
                // Make it a warning only for the time beeing.
                error = FALSE;
            }
            else
            {
                (*tdi)->setTaskRef(t);
                if (t == this)
                {
                    errorMessage(i18n("Task '%1' cannot depend on self.")
                                 .arg(id));
                    error = TRUE;
                    break;
                }
                previous.append(t);
                t->successors.append(this);
                t->followers.append(this);
                if (DEBUGPF(11))
                    qDebug("Registering follower %s with task %s",
                           id.latin1(), t->getId().latin1());
            }
        }
    }

    for (QPtrListIterator<TaskDependency> tdi(precedes); *tdi; ++tdi)
    {
        QString absId = resolveId((*tdi)->getTaskRefId());
        Task* t;
        if ((t = hash.find(absId)) == 0)
        {
            errorMessage(i18n("Unknown dependency '%1'").arg(absId));
            error = TRUE;
        }
        else
        {
            for (QPtrListIterator<TaskDependency> tdi2(precedes); *tdi2; ++tdi2)
                if ((*tdi2)->getTaskRef() == t)
                {
                    errorMessage(i18n("No need to specify dependency '%1'")
                                 .arg(absId));
                    error = TRUE;
                    break;
                }
            if (error)
            {
                // Make it a warning only for the time beeing.
                error = FALSE;
            }
            else
            {
                (*tdi)->setTaskRef(t);
                if (t == this)
                {
                    errorMessage(i18n("Task '%1' cannot precede self.")
                                 .arg(id));
                    error = TRUE;
                    break;
                }
                followers.append(t);
                t->predecessors.append(this);
                t->previous.append(this);
                if (DEBUGPF(11))
                    qDebug("Registering predecessor %s with task %s",
                           id.latin1(), t->getId().latin1());
            }
        }
    }

    return !error;
}

void
Task::implicitXRef()
{
    if (!sub->isEmpty())
        return;

    for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
    {
        /* Propagate implicit dependencies. If a task has no specified start or
         * end date and no start or end dependencies, we check if a parent task
         * has an explicit start or end date which can be used. */

        if (milestone)
        {
            if (scenarios[sc].specifiedStart != 0 &&
                scenarios[sc].specifiedEnd == 0)
                scenarios[sc].specifiedEnd = scenarios[sc].specifiedStart - 1;
            if (scenarios[sc].specifiedEnd != 0 &&
                scenarios[sc].specifiedStart == 0)
                scenarios[sc].specifiedStart = scenarios[sc].specifiedEnd + 1;
        }
        bool hasDurationSpec = scenarios[sc].duration != 0 ||
            scenarios[sc].length != 0 ||
            scenarios[sc].effort != 0;

        if (scenarios[sc].specifiedStart == 0 && depends.isEmpty() &&
            !(hasDurationSpec && scheduling == ALAP))
            for (Task* tp = getParent(); tp; tp = tp->getParent())
            {
                if (tp->scenarios[sc].specifiedStart != 0)
                {
                    if (DEBUGPF(11))
                        qDebug("Setting start of task '%s' in scenario %s to "
                               "%s", id.latin1(),
                               project->getScenarioId(sc).latin1(),
                               time2ISO(tp->scenarios[sc].specifiedStart)
                               .latin1());
                    scenarios[sc].specifiedStart =
                        tp->scenarios[sc].specifiedStart;
                    break;
                }
            }
        /* And the same for end values */
        if (scenarios[sc].specifiedEnd == 0 && precedes.isEmpty() &&
            !(hasDurationSpec && scheduling == ASAP))
            for (Task* tp = getParent(); tp; tp = tp->getParent())
            {
                if (tp->scenarios[sc].specifiedEnd != 0)
                {
                    if (DEBUGPF(11))
                        qDebug("Setting end of task '%s' in scenario %s to %s",
                               id.latin1(),
                               project->getScenarioId(sc).latin1(),
                               time2ISO(tp->scenarios[sc].specifiedEnd)
                               .latin1());
                    scenarios[sc].specifiedEnd = tp->scenarios[sc].specifiedEnd;
                    break;
                }
            }
    }

    if (!isMilestone() && isLeaf())
    {
        /* Automatic milestone marker. As a convenience we convert tasks that
         * only have a start or end criteria as a milestone. This is handy
         * when in the early stage of a project draft, when you just want to
         * specify the project outline and fill in subtasks and details
         * later. */
        bool hasStartSpec = FALSE;
        bool hasEndSpec = FALSE;
        bool hasDurationSpec = FALSE;
        for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
        {
            if (scenarios[sc].specifiedStart != 0 || !depends.isEmpty())
                hasStartSpec = TRUE;
            if (scenarios[sc].specifiedEnd != 0 || !precedes.isEmpty())
                hasEndSpec = TRUE;
            if (scenarios[sc].duration != 0 || scenarios[sc].length != 0 ||
                scenarios[sc].effort != 0)
                hasDurationSpec = TRUE;
        }
        if  (!hasDurationSpec && (hasStartSpec ^ hasEndSpec))
            milestone = TRUE;
    }
}

void
Task::saveSpecifiedBookedResources()
{
    /* The project file readers use the same resource booking mechanism as the
     * scheduler. So we need to save the up to now booked resources as
     * specified resources. */
    for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
        scenarios[sc].specifiedBookedResources =
            scenarios[sc].bookedResources;
}

bool
Task::loopDetector()
{
    /* Only check top-level tasks. All other tasks will be checked then as
     * well. */
    if (parent)
        return FALSE;
    if (DEBUGPF(2))
        qDebug("Running loop detector for task %s", id.latin1());
    // Check ASAP tasks
    LDIList list;
    if (loopDetection(list, FALSE, LoopDetectorInfo::fromParent))
        return TRUE;
    // Check ALAP tasks
    if (loopDetection(list, TRUE, LoopDetectorInfo::fromParent))
        return TRUE;
    return FALSE;
}

bool
Task::loopDetection(LDIList& list, bool atEnd, LoopDetectorInfo::FromWhere
                    caller)
{
    if (DEBUGPF(10))
        qDebug("%sloopDetection at %s (%s)",
               QString().fill(' ', list.count() + 1).latin1(), id.latin1(),
               atEnd ? "End" : "Start");

    /* If we find the current task (with same position) in the list, we have
     * detected a loop. */
    LoopDetectorInfo* thisTask = new LoopDetectorInfo(this, atEnd);
    if ((atEnd && loopDetectorMarkEnd) || (!atEnd && loopDetectorMarkStart))
    {
        QString loopChain;
        LoopDetectorInfo* it;
        for (it = list.first(); *it != *thisTask; it =
             it->next())
            ;
        for ( ; it != 0; it = it->next())
        {
            loopChain += QString("%1 (%2) -> ")
                .arg(it->getTask()->getId())
                .arg(it->getAtEnd() ? "End" : "Start");
        }
        loopChain += QString("%1 (%2)").arg(id)
            .arg(atEnd ? "End" : "Start");
        delete thisTask;
        errorMessage(i18n("Dependency loop detected: %1").arg(loopChain));
        return TRUE;
    }
    list.append(thisTask);

    /* Now we have to traverse the graph in the direction of the specified
     * dependencies. 'precedes' and 'depends' specify dependencies in the
     * opposite direction of the flow of the tasks. So we have to make sure
     * that we do not follow the arcs in the direction that precedes and
     * depends points us. Parent/Child relationships also specify a
     * dependency. The scheduling mode of the child determines the direction
     * of the flow. With help of the 'caller' parameter we make sure that we
     * only visit childs if we were referred to the task by a non-parent-child
     * relationship. */
    if (!atEnd)
    {
        loopDetectorMarkStart = TRUE;
        /*
             |
             v
           +--------
        -->| o--+
           +--- | --
                |
                V
        */
        if (caller == LoopDetectorInfo::fromPrev ||
            caller == LoopDetectorInfo::fromParent)
            /* If we were not called from a sub task we check all sub tasks.*/
            for (TaskListIterator tli(*sub); *tli != 0; ++tli)
            {
                /* If the task depends on a brother we ignore the arc to the
                 * child since the brother provides an equivalent arc. This
                 * can significantly decrease the number of checked pathes
                 * without missing loops. */
                if (!(*tli)->dependsOnABrother(this))
                {
                    if (DEBUGPF(15))
                        qDebug("%sChecking sub task %s of %s",
                               QString().fill(' ', list.count()).latin1(),
                               (*tli)->getId().latin1(),
                               id.latin1());
                    if ((*tli)->loopDetection(list, FALSE,
                                              LoopDetectorInfo::fromParent))
                        return TRUE;
                }
            }

        /*
             |
             v
           +--------
        -->| o---->
           +--------
        */
        if (scheduling == ASAP && sub->isEmpty())
        {
            /* Leaf task are followed in their scheduling direction. So we
             * move from the task start to the task end. */
            if (DEBUGPF(15))
                qDebug("%sChecking end of task %s",
                       QString().fill(' ', list.count()).latin1(),
                       id.latin1());
            if (loopDetection(list, TRUE, LoopDetectorInfo::fromOtherEnd))
                return TRUE;
        }
        if (caller == LoopDetectorInfo::fromSub ||
            caller == LoopDetectorInfo::fromOtherEnd ||
            (caller == LoopDetectorInfo::fromPrev && scheduling == ALAP))
        {
            /*
                 ^
                 |
               + | -----
            -->| o <--
               +--------
                 ^
                 |
            */
            if (parent)
            {
                /* If the task depends on a brother we ignore the arc to the
                 * parent since the brother provides an equivalent arc. This
                 * can significantly decrease the number of checked pathes
                 * without missing loops. */
                if (!dependsOnABrother((const Task*) parent))
                {
                    if (DEBUGPF(15))
                        qDebug("%sChecking parent task of %s",
                               QString().fill(' ', list.count()).latin1(),
                               id.latin1());
                    if (getParent()->loopDetection(list, FALSE,
                                                   LoopDetectorInfo::fromSub))
                        return TRUE;
                }
            }

            /*
             <---+
               + | -----
            -->| o <--
               +--------
                 ^
                 |
            */
            /* Now check all previous tasks that had explicit precedes on this
             * task. */
            for (TaskListIterator tli(predecessors); *tli != 0; ++tli)
            {
                /* If the parent of the predecessor is also in the predecessor
                 * list, we can safely ignore the predecessor. */
                if (predecessors.findRef((*tli)->parent) == -1)
                {
                    if (DEBUGPF(15))
                        qDebug("%sChecking previous %s of task %s",
                               QString().fill(' ', list.count()).latin1(),
                               (*tli)->getId().latin1(), id.latin1());
                    if((*tli)->loopDetection(list, TRUE,
                                             LoopDetectorInfo::fromSucc))
                        return TRUE;
                }
            }
        }
        loopDetectorMarkStart = FALSE;
    }
    else
    {
        loopDetectorMarkEnd = TRUE;
        /*
              |
              v
        --------+
           +--o |<--
        -- | ---+
           |
           v
        */
        if (caller == LoopDetectorInfo::fromSucc ||
            caller == LoopDetectorInfo::fromParent)
            /* If we were not called from a sub task we check all sub tasks.*/
            for (TaskListIterator tli(*sub); *tli != 0; ++tli)
            {
                /* If the task precedes a brother we ignore the arc to the
                 * child since the brother provides an equivalent arc. This
                 * can significantly decrease the number of checked pathes
                 * without missing loops. */
                if (!(*tli)->precedesABrother(this))
                {
                    if (DEBUGPF(15))
                        qDebug("%sChecking sub task %s of %s",
                               QString().fill(' ', list.count()).latin1(),
                               (*tli)->getId().latin1(), id.latin1());
                    if ((*tli)->loopDetection(list, TRUE,
                                              LoopDetectorInfo::fromParent))
                        return TRUE;
                }
            }

        /*
              |
              v
        --------+
         <----o |<--
        --------+
        */
        if (scheduling == ALAP && sub->isEmpty())
        {
            /* Leaf task are followed in their scheduling direction. So we
             * move from the task end to the task start. */
            if (DEBUGPF(15))
                qDebug("%sChecking start of task %s",
                       QString().fill(' ', list.count()).latin1(),
                       id.latin1());
            if (loopDetection(list, FALSE, LoopDetectorInfo::fromOtherEnd))
                return TRUE;
        }
        if (caller == LoopDetectorInfo::fromOtherEnd ||
            caller == LoopDetectorInfo::fromSub ||
            (caller == LoopDetectorInfo::fromSucc && scheduling == ASAP))
        {
            /*
                  ^
                  |
            ----- | +
              --> o |<--
            --------+
                  ^
                  |
            */
            if (parent)
            {
                /* If the task precedes a brother we ignore the arc to the
                 * parent since the brother provides an equivalent arc. This
                 * can significantly decrease the number of checked pathes
                 * without missing loops. */
                if (!precedesABrother((const Task*) parent))
                {
                    if (DEBUGPF(15))
                        qDebug("%sChecking parent task of %s",
                               QString().fill(' ', list.count()).latin1(),
                               id.latin1());
                    if (getParent()->loopDetection(list, TRUE,
                                                   LoopDetectorInfo::fromSub))
                        return TRUE;
                }
            }

            /*
                  +--->
            ----- | +
              --> o |<--
            --------+
                  ^
                  |
            */
            /* Now check all following tasks that have explicit depends on this
             * task. */
            for (TaskListIterator tli(successors); *tli != 0; ++tli)
            {
                /* If the parent of the successor is also in the successor
                 * list, we can safely ignore the successor. */
                if (successors.findRef((*tli)->parent) == -1)
                {
                    if (DEBUGPF(15))
                        qDebug("%sChecking follower %s of task %s",
                               QString().fill(' ', list.count()).latin1(),
                               (*tli)->getId().latin1(), id.latin1());
                    if ((*tli)->loopDetection(list, FALSE,
                                              LoopDetectorInfo::fromPrev))
                        return TRUE;
                }
            }
        }
        loopDetectorMarkEnd = FALSE;
    }
    list.removeLast();

    if (DEBUGPF(5))
        qDebug("%sNo loops found in %s (%s)",
                 QString().fill(' ', list.count()).latin1(),
                 id.latin1(), atEnd ? "End" : "Start");
    return FALSE;
}

QString
Task::resolveId(QString relId)
{
    /* Converts a relative ID to an absolute ID. Relative IDs start
     * with a number of bangs. A set of bangs means 'Name of the n-th
     * parent task' with n being the number of bangs. */
    if (relId[0] != '!')
        return relId;

    Task* t = this;
    unsigned int i;
    for (i = 0; i < relId.length() && relId.mid(i, 1) == "!"; ++i)
    {
        if (t == 0)
        {
            errorMessage(i18n("Illegal relative ID '%1'").arg(relId));
            return relId;
        }
        t = t->getParent();
    }
    if (t)
        return t->id + "." + relId.right(relId.length() - i);
    else
        return relId.right(relId.length() - i);
}

bool
Task::hasStartDependency(int sc)
{
    /* Checks whether the task has a start specification for the
     * scenario. This can be a fixed start time or a dependency on another
     * task's end or an implicit dependency on the fixed start time of a
     * parent task. */
    if (scenarios[sc].specifiedStart != 0 || !depends.isEmpty())
        return TRUE;
    for (Task* p = getParent(); p; p = p->getParent())
        if (p->scenarios[sc].specifiedStart != 0)
            return TRUE;
    return FALSE;
}

bool
Task::hasEndDependency(int sc)
{
    /* Checks whether the task has an end specification for the
     * scenario. This can be a fixed end time or a dependency on another
     * task's start or an implicit dependency on the fixed end time of a
     * parent task. */
    if (scenarios[sc].specifiedEnd != 0 || !precedes.isEmpty())
        return TRUE;
    for (Task* p = getParent(); p; p = p->getParent())
        if (p->scenarios[sc].specifiedEnd != 0)
            return TRUE;
    return FALSE;
}

bool
Task::hasStartDependency()
{
    /* Check whether the task or any of it's sub tasks has a start
     * dependency. */
    if (start != 0 || !previous.isEmpty() || scheduling == ALAP)
        return TRUE;

    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        if ((*tli)->hasStartDependency())
            return TRUE;

    return FALSE;
}

bool
Task::hasEndDependency()
{
    /* Check whether the task or any of it's sub tasks has an end
     * dependency. */
    if (end != 0 || !followers.isEmpty() || scheduling == ASAP)
        return TRUE;

    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        if ((*tli)->hasStartDependency())
            return TRUE;

    return FALSE;
}

bool
Task::preScheduleOk(int sc)
{
    if (scenarios[sc].specifiedScheduled && !sub->isEmpty() &&
        (scenarios[sc].specifiedStart == 0 ||
         scenarios[sc].specifiedEnd == 0))
    {
        errorMessage(i18n
                     ("Task '%1' is marked as scheduled but does not have "
                      "a fixed start and end date.").arg(id));
        return FALSE;
    }

    if (scenarios[sc].effort > 0.0 && allocations.count() == 0 &&
        !scenarios[sc].specifiedScheduled)
    {
        errorMessage(i18n
                     ("No allocations specified for effort based task '%1' "
                      "in '%2' scenario")
                     .arg(id).arg(project->getScenarioId(sc)));
        return FALSE;
    }

    if (scenarios[sc].startBuffer + scenarios[sc].endBuffer >= 100.0)
    {
        errorMessage(i18n
                     ("Start and end buffers may not overlap in '%2' "
                      "scenario. So their sum must be smaller then 100%.")
                     .arg(project->getScenarioId(sc)));
        return FALSE;
    }

    // Check plan values.
    int durationSpec = 0;
    if (scenarios[sc].effort > 0.0)
        durationSpec++;
    if (scenarios[sc].length > 0.0)
        durationSpec++;
    if (scenarios[sc].duration > 0.0)
        durationSpec++;
    if (durationSpec > 1)
    {
        errorMessage(i18n("Task '%1' may only have one duration "
                          "criteria in '%2' scenario.").arg(id)
                     .arg(project->getScenarioId(sc)));
        return FALSE;
    }

    /*
    |: fixed start or end date
    -: no fixed start or end date
    M: Milestone
    D: start or end dependency
    x->: ASAP task with duration criteria
    <-x: ALAP task with duration criteria
    -->: ASAP task without duration criteria
    <--: ALAP task without duration criteria
     */
    if (!sub->isEmpty())
    {
        if (durationSpec != 0)
        {
            errorMessage(i18n
                         ("Container task '%1' may not have a duration "
                          "criteria in '%2' scenario").arg(id)
                         .arg(project->getScenarioId(sc)));
            return FALSE;
        }
        if (milestone)
        {
            errorMessage(i18n
                         ("The container task '%1' may not be a "
                          "milestone.").arg(id));
            return FALSE;
        }
    }
    else if (milestone)
    {
        if (durationSpec != 0)
        {
            errorMessage(i18n
                         ("Milestone '%1' may not have a duration "
                          "criteria in '%2' scenario").arg(id)
                         .arg(project->getScenarioId(sc)));
            return FALSE;
        }
        /*
        |  M -   ok     |D M -   ok     - M -   err1   -D M -   ok
        |  M |   err2   |D M |   err2   - M |   ok     -D M |   ok
        |  M -D  ok     |D M -D  ok     - M -D  ok     -D M -D  ok
        |  M |D  err2   |D M |D  err2   - M |D  ok     -D M |D  ok
         */
        /* err1: no start and end
        - M -
         */
        if (!hasStartDependency(sc) && !hasEndDependency(sc))
        {
            errorMessage(i18n("Milestone '%1' must have a start or end "
                              "specification in '%2' scenario.")
                         .arg(id).arg(project->getScenarioId(sc)));
            return FALSE;
        }
        /* err2: different start and end
        |  M |
        |  M |D
        |D M |
        |D M |D
         */
        if (scenarios[sc].specifiedStart != 0 &&
            scenarios[sc].specifiedEnd != 0 &&
            scenarios[sc].specifiedStart != scenarios[sc].specifiedEnd + 1)
        {
            errorMessage(i18n
                         ("Milestone '%1' may not have both a start "
                          "and an end specification that do not "
                          "match in the '%2' scenario.").arg(id)
                         .arg(project->getScenarioId(sc)));
            return FALSE;
        }
    }
    else
    {
        /*
        Error table for non-container, non-milestone tasks:

        | x-> -   ok     |D x-> -   ok     - x-> -   err3   -D x-> -   ok
        | x-> |   err1   |D x-> |   err1   - x-> |   err3   -D x-> |   err1
        | x-> -D  ok     |D x-> -D  ok     - x-> -D  err3   -D x-> -D  ok
        | x-> |D  err1   |D x-> |D  err1   - x-> |D  err3   -D x-> |D  err1
        | --> -   err2   |D --> -   err2   - --> -   err3   -D --> -   err2
        | --> |   ok     |D --> |   ok     - --> |   err3   -D --> |   ok
        | --> -D  ok     |D --> -D  ok     - --> -D  err3   -D --> -D  ok
        | --> |D  ok     |D --> |D  ok     - --> |D  err3   -D --> |D  ok
        | <-x -   err4   |D <-x -   err4   - <-x -   err4   -D <-x -   err4
        | <-x |   err1   |D <-x |   err1   - <-x |   ok     -D <-x |   ok
        | <-x -D  err1   |D <-x -D  err1   - <-x -D  ok     -D <-x -D  ok
        | <-x |D  err1   |D <-x |D  err1   - <-x |D  ok     -D <-x |D  ok
        | <-- -   err4   |D <-- -   err4   - <-- -   err4   -D <-- -   err4
        | <-- |   ok     |D <-- |   ok     - <-- |   err2   -D <-- |   ok
        | <-- -D  ok     |D <-- -D  ok     - <-- -D  err2   -D <-- -D  ok
        | <-- |D  ok     |D <-- |D  ok     - <-- |D  err2   -D <-- |D  ok
         */
        /*
        err1: Overspecified (12 cases)
        |  x-> |
        |  <-x |
        |  x-> |D
        |  <-x |D
        |D x-> |
        |D <-x |
        |D <-x |D
        |D x-> |D
        -D x-> |
        -D x-> |D
        |D <-x -D
        |  <-x -D
         */
        if (((scenarios[sc].specifiedStart != 0 &&
              scenarios[sc].specifiedEnd != 0) ||
             (hasStartDependency(sc) && scenarios[sc].specifiedStart == 0 &&
              scenarios[sc].specifiedEnd != 0 && scheduling == ASAP) ||
             (scenarios[sc].specifiedStart != 0 && scheduling == ALAP &&
              hasEndDependency(sc) && scenarios[sc].specifiedEnd == 0)) &&
            durationSpec != 0 && !scenarios[sc].specifiedScheduled)
        {
            errorMessage(i18n("Task '%1' has a start, an end and a "
                              "duration specification for '%2' scenario.")
                         .arg(id).arg(project->getScenarioId(sc)));
            return FALSE;
        }
        /*
        err2: Underspecified (6 cases)
        |  --> -
        |D --> -
        -D --> -
        -  <-- |
        -  <-- |D
        -  <-- -D
         */
        if ((hasStartDependency(sc) ^ hasEndDependency(sc)) &&
            durationSpec == 0)
        {
            errorMessage(i18n
                         ("Task '%1' has only a start or end specification "
                          "but no duration for the '%2' scenario.")
                         .arg(id).arg(project->getScenarioId(sc)));
            return FALSE;
        }
        /*
        err3: ASAP + Duration must have fixed start (8 cases)
        -  x-> -
        -  x-> |
        -  x-> -D
        -  x-> |D
        -  --> -
        -  --> |
        -  --> -D
        -  --> |D
         */
        if (!hasStartDependency(sc) && scheduling == ASAP)
        {
            errorMessage(i18n
                         ("Task '%1' needs a start specification to be "
                          "scheduled in ASAP mode in the '%2' scenario.")
                         .arg(id).arg(project->getScenarioId(sc)));
            return FALSE;
        }
        /*
        err4: ALAP + Duration must have fixed end (8 cases)
        -  <-x -
        |  <-x -
        |D <-x -
        -D <-x -
        -  <-- -
        |  <-- -
        -D <-- -
        |D <-- -
         */
        if (!hasEndDependency(sc) && scheduling == ALAP)
        {
            errorMessage(i18n
                         ("Task '%1' needs an end specification to be "
                          "scheduled in ALAP mode in the '%2' scenario.")
                         .arg(id).arg(project->getScenarioId(sc)));
            return FALSE;
        }
  }

  if (!account &&
      (scenarios[sc].startCredit > 0.0 || scenarios[sc].endCredit > 0.0))
  {
      errorMessage(i18n
                   ("Task '%1' has a specified start- or endcredit "
                    "but no account assigned in scenario '%2'.")
                   .arg(id).arg(project->getScenarioId(sc)));
      return FALSE;
  }

  if (!scenarios[sc].bookedResources.isEmpty() && scheduling == ALAP &&
      !scenarios[sc].specifiedScheduled)
  {
      errorMessage
          (i18n("Error in task '%1' (scenario '%2'). "
                "An ALAP task can only have bookings if it has been "
                "completely scheduled. The 'scheduled' attribute must be "
                "present. Keep in mind that certain attributes such as "
                "'precedes' or 'end' implicitly set the scheduling mode "
                "to ALAP. Put 'scheduling asap' at the end of the task "
                "definition to avoid the problem.")
           .arg(id).arg(project->getScenarioId(sc)));
      return FALSE;
  }

  return TRUE;
}

bool
Task::scheduleOk(int sc, int& errors) const
{
    const QString scenario = project->getScenarioId(sc);

    /* It is of little use to report errors of container tasks, if any of
     * their sub tasks has errors. */
    int currErrors = errors;
    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        (*tli)->scheduleOk(sc, errors);
    if (errors > currErrors)
    {
        if (DEBUGPS(2))
            qDebug(QString("Scheduling errors in sub tasks of '%1'.")
                   .arg(id));
        return FALSE;
    }

    /* Runaway errors have already been reported. Since the data of this task
     * is very likely completely bogus, we just return FALSE. */
    if (runAway)
        return FALSE;

    if (DEBUGPS(3))
        qDebug("Checking task %s", id.latin1());

    /* If any of the dependant tasks is a runAway, we can safely surpress all
     * other error messages. */
    for (QPtrListIterator<TaskDependency> tdi(depends); *tdi; ++tdi)
        if ((*tdi)->getTaskRef()->runAway)
            return FALSE;
    for (QPtrListIterator<TaskDependency> tdi(precedes); *tdi; ++tdi)
        if ((*tdi)->getTaskRef()->runAway)
            return FALSE;

    if (start == 0)
    {
        errorMessage(i18n("Task '%1' has no start time for the '%2'"
                          "scenario.")
                     .arg(id).arg(scenario));
        errors++;
        return FALSE;
    }
    if (start < project->getStart() || start > project->getEnd())
    {
        errorMessage(i18n("Start time '%1' of task '%2' is outside of the "
                          "project interval (%3 - %4) in '%5' scenario.")
                     .arg(time2tjp(start))
                     .arg(id)
                     .arg(time2tjp(project->getStart()))
                     .arg(time2tjp(project->getEnd()))
                     .arg(scenario));
        errors++;
        return FALSE;
    }
    if (scenarios[sc].minStart != 0 && start < scenarios[sc].minStart)
    {
        errorMessage(i18n("'%1' start time of task '%2' is too early\n"
                          "Date is:  %3\n"
                          "Limit is: %4")
                     .arg(scenario).arg(id).arg(time2tjp(start))
                     .arg(time2tjp(scenarios[sc].minStart)));
        errors++;
        return FALSE;
    }
    if (scenarios[sc].maxStart != 0 && start > scenarios[sc].maxStart)
    {
        errorMessage(i18n("'%1' start time of task '%2' is too late\n"
                          "Date is:  %3\n"
                          "Limit is: %4")
                     .arg(scenario).arg(id)
                     .arg(time2tjp(start))
                     .arg(time2tjp(scenarios[sc].maxStart)));
        errors++;
        return FALSE;
    }
    if (end == 0)
    {
        errorMessage(i18n("Task '%1' has no '%2' end time.")
                     .arg(id).arg(scenario.lower()));
        errors++;
        return FALSE;
    }
    if ((end + 1) < project->getStart() || (end > project->getEnd()))
    {
        errorMessage(i18n("End time '%1' of task '%2' is outside of the "
                          "project interval (%3 - %4) in '%5' scenario.")
                     .arg(time2tjp(end + 1))
                     .arg(id)
                     .arg(time2tjp(project->getStart()))
                     .arg(time2tjp(project->getEnd() + 1))
                     .arg(scenario));
        errors++;
        return FALSE;
    }
    if (scenarios[sc].minEnd != 0 && end < scenarios[sc].minEnd)
    {
        errorMessage(i18n("'%1' end time of task '%2' is too early\n"
                          "Date is:  %3\n"
                          "Limit is: %4")
                     .arg(scenario).arg(id)
                     .arg(time2tjp(end + 1))
                     .arg(time2tjp(scenarios[sc].minEnd + 1)));
        errors++;
        return FALSE;
    }
    if (scenarios[sc].maxEnd != 0 && end > scenarios[sc].maxEnd)
    {
        errorMessage(i18n("'%1' end time of task '%2' is too late\n"
                          "Date is:  %2\n"
                          "Limit is: %3")
                     .arg(scenario).arg(id)
                     .arg(time2tjp(end + 1))
                     .arg(time2tjp(scenarios[sc].maxEnd + 1)));
        errors++;
        return FALSE;
    }
    if (!sub->isEmpty())
    {
        // All sub task must fit into their parent task.
        for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        {
            if (start > (*tli)->start)
            {
                if (!(*tli)->runAway)
                {
                    errorMessage(i18n("Task '%1' has earlier '%2' start than "
                                      "parent\n"
                                      "%3 start date: %4\n"
                                      "%5 start date: %6")
                                 .arg((*tli)->getId()).arg(scenario)
                                 .arg(id.latin1())
                                 .arg(time2ISO(start).latin1())
                                 .arg((*tli)->getId().latin1())
                                 .arg(time2ISO((*tli)->start).latin1()));
                    errors++;
                }
                return FALSE;
            }
            if (end < (*tli)->end)
            {
                if (!(*tli)->runAway)
                {
                    errorMessage(i18n("Task '%1' has later '%2' end than "
                                      "parent")
                                 .arg(id).arg(scenario));
                    errors++;
                }
                return FALSE;
            }
        }
    }

    // Check if all previous tasks end before start of this task.
    for (TaskListIterator tli(previous); *tli != 0; ++tli)
        if ((*tli)->end > start && !(*tli)->runAway)
        {
            errorMessage(i18n("Impossible dependency:\n"
                              "Task '%1' ends at %2 but needs to precede\n"
                              "task '%3' which has a '%4' start time of %5")
                         .arg((*tli)->id).arg(time2tjp((*tli)->end).latin1())
                         .arg(id).arg(scenario).arg(time2tjp(start)));
            errors++;
            return FALSE;
        }
    // Check if all following task start after this tasks end.
    for (TaskListIterator tli(followers); *tli != 0; ++tli)
        if (end > (*tli)->start && !(*tli)->runAway)
        {
            errorMessage(i18n("Impossible dependency:\n"
                              "Task '%1' starts at %2 but needs to follow\n"
                              "task %3 which has a '%4' end time of %5")
                         .arg((*tli)->id).arg(time2tjp((*tli)->start))
                         .arg(id).arg(scenario).arg(time2tjp(end)));
            errors++;
            return FALSE;
        }

    if (!schedulingDone)
    {
        errorMessage(i18n("Task '%1' has not been marked completed.\n"
                          "It is scheduled to last from %2 to %3.\n"
                          "This might be a bug in the TaskJuggler scheduler.")
                     .arg(id).arg(time2tjp(start)).arg(time2tjp(end)));
        errors++;
        return FALSE;
    }

    return TRUE;
}

time_t
Task::nextSlot(time_t slotDuration) const
{
    /* This function returns the start of the next time slot this task wants
     * to be scheduled in. If there is no such slot because the tasks does not
     * yet have all necessary information, 0 is returned. This also happens
     * when the task has already be completely scheduled. */
    if (schedulingDone)
        return 0;

    if (scheduling == ASAP)
    {
        if (start != 0)
        {
            if (effort == 0.0 && length == 0.0 && duration == 0.0 &&
                !milestone && end == 0)
                return 0;

            if (lastSlot == 0)
                return start;
            return lastSlot + 1;
        }
        else
            return 0;
    }
    else
    {
        if (end != 0)
        {
            if (effort == 0.0 && length == 0.0 && duration == 0.0 &&
                !milestone && start == 0)
                return 0;

            if (lastSlot == 0)
                return end - slotDuration + 1;
            return lastSlot - slotDuration;
        }
        else
            return 0;
    }

    return 0;
}

bool
Task::isActive(int sc, const Interval& period) const
{
    return period.overlaps(Interval(scenarios[sc].start,
                                    milestone ? scenarios[sc].start :
                                    scenarios[sc].end));
}

bool
Task::isSubTask(Task* tsk) const
{
    for (TaskListIterator tli(*sub); *tli != 0; ++tli)
        if (*tli == tsk || (*tli)->isSubTask(tsk))
            return TRUE;

    return FALSE;
}

void
Task::overlayScenario(int base, int sc)
{
    /* Copy all values that the scenario sc does not provide, but that are
     * provided by the base scenario to the scenario sc. */
    if (scenarios[sc].specifiedStart == 0.0)
        scenarios[sc].specifiedStart = scenarios[base].specifiedStart;
    if (scenarios[sc].specifiedEnd == 0.0)
        scenarios[sc].specifiedEnd = scenarios[base].specifiedEnd;
    if (scenarios[sc].minStart == 0.0)
        scenarios[sc].minStart = scenarios[base].minStart;
    if (scenarios[sc].maxStart == 0.0)
        scenarios[sc].maxStart = scenarios[base].maxStart;
    if (scenarios[sc].minEnd == 0.0)
        scenarios[sc].minEnd = scenarios[base].minEnd;
    if (scenarios[sc].maxEnd == 0.0)
        scenarios[sc].maxEnd = scenarios[base].maxEnd;
    if (scenarios[sc].duration == 0.0)
        scenarios[sc].duration =  scenarios[base].duration;
    if (scenarios[sc].length == 0.0)
        scenarios[sc].length = scenarios[base].length;
    if (scenarios[sc].effort == 0.0)
        scenarios[sc].effort = scenarios[base].effort;
    if (scenarios[sc].startBuffer < 0.0)
        scenarios[sc].startBuffer = scenarios[base].startBuffer;
    if (scenarios[sc].endBuffer < 0.0)
        scenarios[sc].endBuffer = scenarios[base].endBuffer;
    if (scenarios[sc].startCredit < 0.0)
        scenarios[sc].startCredit = scenarios[base].startCredit;
    if (scenarios[sc].endCredit < 0.0)
        scenarios[sc].endCredit = scenarios[base].endCredit;
    if (scenarios[sc].reportedCompletion < 0.0)
        scenarios[sc].reportedCompletion = scenarios[base].reportedCompletion;
}

bool
Task::hasExtraValues(int sc) const
{
    return scenarios[sc].start != 0 || scenarios[sc].end != 0 ||
        scenarios[sc].length != 0 || scenarios[sc].duration != 0 ||
        scenarios[sc].effort != 0 || scenarios[sc].reportedCompletion >= 0.0 ||
        scenarios[sc].startBuffer >= 0.0 || scenarios[sc].endBuffer >= 0.0 ||
        scenarios[sc].startCredit >= 0.0 || scenarios[sc].endCredit >= 0.0;
}

void
Task::prepareScenario(int sc)
{
    start = scenarios[sc].start = scenarios[sc].specifiedStart;
    end = scenarios[sc].end = scenarios[sc].specifiedEnd;
    schedulingDone = scenarios[sc].scheduled = scenarios[sc].specifiedScheduled;

    duration = scenarios[sc].duration;
    length = scenarios[sc].length;
    effort = scenarios[sc].effort;
    lastSlot = 0;
    doneEffort = 0.0;
    doneDuration = 0.0;
    doneLength = 0.0;
    tentativeStart = tentativeEnd = 0;
    workStarted = FALSE;
    runAway = FALSE;
    bookedResources.clear();
    bookedResources = scenarios[sc].specifiedBookedResources;

    /* The user could have made manual bookings already. The effort of these
     * bookings needs to be calculated so that the scheduler only schedules
     * the still missing effort. Scheduling will begin after the last booking.
     * This will only work for ASAP tasks. ALAP tasks cannot be partly booked.
     */
    time_t firstSlot = 0;
    for (ResourceListIterator rli(bookedResources); *rli != 0; ++rli)
    {
        doneEffort += (*rli)->getLoad(sc,
                                      Interval(project->getStart(),
                                               project->getEnd()),
                                      AllAccounts, this);
        if (doneEffort > 0.0)
        {
            if (firstSlot == 0 ||
                firstSlot > (*rli)->getStartOfFirstSlot(sc, this))
            {
                firstSlot = (*rli)->getStartOfFirstSlot(sc, this);
            }
            time_t ls = (*rli)->getEndOfLastSlot(sc, this);
            if (ls > lastSlot)
                lastSlot = ls;
        }
    }
    if (lastSlot > 0 && !schedulingDone)
    {
        workStarted = TRUE;
        // Trim start to first booked time slot.
        start = firstSlot;

        /* In projection mode, we assume that the completed work has been
         * reported with booking attributes. Now we compute the completion
         * degree according to the overall effort. Then the end date of the
         * task is calculated. */
        if (project->getScenario(sc)->getProjectionMode() && effort > 0.0)
            scenarios[sc].reportedCompletion = doneEffort / effort * 100.0;
    }

    /*
     * To determine the criticalness of an effort based task, we need to
     * determine the allocation probability of all of the resources. The more
     * the resources that are allocated to a task are allocated the smaller is
     * the likelyhood that the task will get it's allocation, the more
     * critical it is.
     *
     * The allocation probability of a resource is basically effort divided by
     * number of allocated resources. Since the efficiency of resources can
     * vary we need to determine the overall efficiency first.
     *
     * TODO: We need to respect limits and shifts here!
     */
    double allocationEfficiency = 0;
    for (QPtrListIterator<Allocation> ali(allocations); *ali != 0; ++ali)
    {
        (*ali)->init();
        if (scenarios[sc].effort > 0.0)
        {
            double maxEfficiency = 0;
            for (QPtrListIterator<Resource> rli =
                 (*ali)->getCandidatesIterator(); *rli; ++rli)
            {
                for (ResourceTreeIterator rti(*rli); *rti; ++rti)
                    if ((*rti)->getEfficiency() > maxEfficiency)
                        maxEfficiency = (*rti)->getEfficiency();
            }
            allocationEfficiency += maxEfficiency;
        }
    }
    if (scenarios[sc].effort > 0.0)
    {
        /* Now we can add the allocation probability for this task to all the
         * individual resources. */
        double effortPerResource = effort / allocationEfficiency;
        for (QPtrListIterator<Allocation> ali(allocations); *ali != 0; ++ali)
            for (QPtrListIterator<Resource> rli =
                 (*ali)->getCandidatesIterator(); *rli; ++rli)
                for (ResourceTreeIterator rti(*rli); *rti; ++rti)
                    (*rti)->addAllocationProbability
                        (sc, effortPerResource * (*rti)->getEfficiency());
    }
}

void
Task::computeCriticalness(int sc)
{
    if (scenarios[sc].effort > 0.0)
    {
        double overallAllocationProbability = 0;
        for (QPtrListIterator<Allocation> ali(allocations); *ali != 0; ++ali)
        {
            /* We assume that out of the candidates for an allocation the
             * one with the smallest overall allocation probability will
             * be assigned to the task. */
            double smallestAllocationProbablity = 0;
            for (QPtrListIterator<Resource> rli =
                 (*ali)->getCandidatesIterator(); *rli; ++rli)
            {
                /* If the candidate is a resource group we use the average
                 * allocation probablility of all the resources of the group.
                 */
                int resources = 0;
                double averageProbability = 0.0;
                for (ResourceTreeIterator rti(*rli); *rti; ++rti, ++resources)
                    averageProbability +=
                        (*rti)->getAllocationProbability(sc);

                if (smallestAllocationProbablity == 0 ||
                    averageProbability < smallestAllocationProbablity)
                    smallestAllocationProbablity = averageProbability;
            }
            overallAllocationProbability += smallestAllocationProbablity;
        }
        scenarios[sc].criticalness = (scenarios[sc].effort *
            overallAllocationProbability) / allocations.count();
    }
    else
        scenarios[sc].criticalness = 0;

}

void
Task::computePathCriticalness(int sc)
{
    /*
     * The path criticalness is a measure for the overall criticalness of the
     * task taking the dependencies into account. The fact that a task is part
     * of a chain of effort-based task raises all the task in the chain to a
     * higher criticalness level than the individual tasks. In fact, the path
     * criticalness of this chain is equal to the sum of the individual
     * criticalnesses of the tasks.
     */
    if (effort > 0.0)
        /* Since both the forward and backward functions include the
         * criticalness of this function we have to subtract it again. */
        scenarios[sc].pathCriticalness = computeBackwardCriticalness(sc) -
            scenarios[sc].criticalness + computeForwardCriticalness(sc);
    else
        scenarios[sc].pathCriticalness = 0.0;
}

double
Task::computeBackwardCriticalness(int sc)
{
    double maxCriticalness = 0.0;

    double criticalness;
    for (TaskListIterator tli(previous); *tli; ++tli)
        if ((criticalness = (*tli)->computeBackwardCriticalness(sc)) >
            maxCriticalness)
            maxCriticalness = criticalness;
    if (parent &&
        (criticalness = ((Task*) parent)->computeBackwardCriticalness(sc) >
         maxCriticalness))
        maxCriticalness = criticalness;

    return scenarios[sc].criticalness + maxCriticalness;
}

double
Task::computeForwardCriticalness(int sc)
{
    double maxCriticalness = 0.0;

    double criticalness;
    for (TaskListIterator tli(followers); *tli; ++tli)
        if ((criticalness = (*tli)->computeForwardCriticalness(sc)) >
            maxCriticalness)
            maxCriticalness = criticalness;
    if (parent &&
        (criticalness = ((Task*) parent)->computeForwardCriticalness(sc) >
         maxCriticalness))
        maxCriticalness = criticalness;

    return scenarios[sc].criticalness + maxCriticalness;
}

void
Task::finishScenario(int sc)
{
    scenarios[sc].start = start;
    scenarios[sc].end = end;
    scenarios[sc].bookedResources = bookedResources;
    scenarios[sc].scheduled = schedulingDone;

    calcCompletionDegree(sc);
}

void
Task::computeBuffers()
{
    int sg = project->getScheduleGranularity();
    for (int sc = 0; sc < project->getMaxScenarios(); sc++)
    {
        scenarios[sc].startBufferEnd = scenarios[sc].start - 1;
        scenarios[sc].endBufferStart = scenarios[sc].end + 1;

        if (scenarios[sc].start == 0 || scenarios[sc].end == 0)
        {
            scenarios[sc].startBufferEnd = scenarios[sc].endBufferStart = 0;
            continue;
        }

        if (duration > 0.0)
        {
            if (scenarios[sc].startBuffer > 0.0)
                scenarios[sc].startBufferEnd = scenarios[sc].start +
                    (time_t) ((scenarios[sc].end - scenarios[sc].start) *
                              scenarios[sc].startBuffer / 100.0);
            if (scenarios[sc].endBuffer > 0.0)
                scenarios[sc].endBufferStart = scenarios[sc].end -
                    (time_t) ((scenarios[sc].end - scenarios[sc].start) *
                              scenarios[sc].endBuffer / 100.0);
        }
        else if (length > 0.0)
        {
            double l;
            if (scenarios[sc].startBuffer > 0.0)
            {
                for (l = 0.0; scenarios[sc].startBufferEnd < scenarios[sc].end;
                     scenarios[sc].startBufferEnd += sg)
                {
                    if (project->isWorkingDay(scenarios[sc].startBufferEnd))
                    l += (double) sg / ONEDAY;
                    if (l >= scenarios[sc].length *
                        scenarios[sc].startBuffer / 100.0)
                        break;
                }
            }
            if (scenarios[sc].endBuffer > 0.0)
            {
                for (l = 0.0; scenarios[sc].endBufferStart >
                     scenarios[sc].start; scenarios[sc].endBufferStart -= sg)
                {
                    if (project->isWorkingDay(scenarios[sc].endBufferStart))
                        l += (double) sg / ONEDAY;
                    if (l >= scenarios[sc].length *
                        scenarios[sc].endBuffer / 100.0)
                        break;
                }
            }
        }
        else if (effort > 0.0)
        {
            double e;
            if (scenarios[sc].startBuffer > 0.0)
            {
                for (e = 0.0; scenarios[sc].startBufferEnd < scenarios[sc].end;
                     scenarios[sc].startBufferEnd += sg)
                {
                    e += getLoad(sc,
                                 Interval(scenarios[sc].startBufferEnd,
                                          scenarios[sc].startBufferEnd + sg));
                    if (e >= scenarios[sc].effort *
                        scenarios[sc].startBuffer / 100.0)
                        break;
                }
            }
            if (scenarios[sc].endBuffer > 0.0)
            {
                for (e = 0.0; scenarios[sc].endBufferStart >
                     scenarios[sc].start; scenarios[sc].endBufferStart -= sg)
                {
                    e += getLoad(sc,
                                 Interval(scenarios[sc].endBufferStart - sg,
                                          scenarios[sc].endBufferStart));
                    if (e >= scenarios[sc].effort *
                        scenarios[sc].endBuffer / 100.0)
                        break;
                }
            }
        }
    }
}

void
Task::calcCompletionDegree(int sc)
{
    scenarios[sc].calcCompletionDegree(project->getNow());
}

double
Task::getCompletionDegree(int sc) const
{
    if(scenarios[sc].reportedCompletion >= 0.0)
        return(scenarios[sc].reportedCompletion);

    return scenarios[sc].completionDegree;
}

double
Task::getCalcedCompletionDegree(int sc) const
{
    return scenarios[sc].completionDegree;
}

bool
Task::dependsOnABrother(const Task* p) const
{
    for (QPtrListIterator<TaskDependency> tdi(depends); *tdi; ++tdi)
        if ((*tdi)->getTaskRef() && (*tdi)->getTaskRef()->parent == p)
            return TRUE;
    return FALSE;
}

bool
Task::precedesABrother(const Task* p) const
{
    for (QPtrListIterator<TaskDependency> tdi(precedes); *tdi; ++tdi)
        if ((*tdi)->getTaskRef() && (*tdi)->getTaskRef()->parent == p)
            return TRUE;
    return FALSE;
}

QDomElement Task::xmlElement( QDomDocument& doc, bool /* absId */ )
{
   QDomElement taskElem = doc.createElement( "Task" );
   QDomElement tempElem;

   QString idStr = getId();
/*   if( !absId )
      idStr = idStr.section( '.', -1 ); */

   taskElem.setAttribute( "Id", idStr );

   QDomText t;
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Index", QString::number(getIndex()) ));
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Name", getName() ));
   taskElem.appendChild( ReportXML::createXMLElem( doc, "ProjectID", projectId ));
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Priority", QString::number(getPriority())));

   double cmplt = getCompletionDegree(0);
   taskElem.appendChild( ReportXML::createXMLElem( doc, "complete", QString::number(cmplt, 'f', 1) ));

   QString tType = "Milestone";
   if( !isMilestone() )
   {
      if( isContainer() )
     tType = "Container";
      else
     tType = "Task";

   }
   taskElem.appendChild( ReportXML::createXMLElem( doc, "Type", tType  ));

   CoreAttributes *parent = getParent();
   if( parent )
      taskElem.appendChild( ReportXML::ReportXML::createXMLElem( doc, "ParentTask", parent->getId()));

   if( !note.isEmpty())
      taskElem.appendChild( ReportXML::createXMLElem( doc, "Note", getNote()));
   if(!ref.isEmpty())
       taskElem.appendChild(ReportXML::createXMLElem(doc, "Reference",
                                                     ref));
   if(!refLabel.isEmpty())
       taskElem.appendChild(ReportXML::createXMLElem(doc, "ReferenceLabel",
                                                     refLabel));

    if (scenarios[0].minStart != 0)
    {
        tempElem = ReportXML::createXMLElem
            ( doc, "minStart", QString::number(scenarios[0].minStart));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[0].minStart));
        taskElem.appendChild( tempElem );
    }

    if (scenarios[0].maxStart != 0)
    {
        tempElem = ReportXML::createXMLElem
            (doc, "maxStart", QString::number(scenarios[0].maxStart));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[0].maxStart));
        taskElem.appendChild( tempElem );
    }

    if (scenarios[0].minEnd != 0)
    {
        tempElem = ReportXML::createXMLElem
            (doc, "minEnd", QString::number(scenarios[0].minEnd));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[0].minEnd));
        taskElem.appendChild( tempElem );
    }

    if (scenarios[0].maxEnd != 0)
    {
        tempElem = ReportXML::createXMLElem
            (doc, "maxEnd", QString::number(scenarios[0].maxEnd));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[0].maxEnd));
        taskElem.appendChild( tempElem );
    }
    if (project->getMaxScenarios() > 1)
    {
        tempElem = ReportXML::createXMLElem( doc, "actualStart",
                                             QString::number(scenarios[1].start));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[1].start));
        taskElem.appendChild( tempElem );

        tempElem = ReportXML::createXMLElem( doc, "actualEnd",
                                             QString::number(scenarios[1].end + 1));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[1].end + 1));
        taskElem.appendChild( tempElem );
    }

   tempElem = ReportXML::createXMLElem( doc, "planStart", QString::number( scenarios[0].start ));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[0].start ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "planEnd",
                                        QString::number(scenarios[0].end + 1));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[0].end + 1));
   taskElem.appendChild( tempElem );

   /* Start- and Endbuffer */
   if(getStartBuffer(0) > 0.01)
   {
       /* startbuffer exists */
       tempElem = ReportXML::createXMLElem
           (doc, "startBufferSize",
            QString::number(getStartBuffer(0)));
       taskElem.appendChild( tempElem );

       tempElem = ReportXML::createXMLElem
           (doc, "PlanStartBufferEnd",
            QString::number(getStartBufferEnd(0)));
       tempElem.setAttribute("humanReadable",
                             time2ISO(getStartBufferEnd(0)));
       taskElem.appendChild(tempElem);

       tempElem = ReportXML::createXMLElem
           (doc, "PlanStartBufferEnd",
            QString::number(getStartBufferEnd(0)));
       tempElem.setAttribute("humanReadable",
                             time2ISO(getStartBufferEnd(0)));
       taskElem.appendChild(tempElem);
   }

   if(getEndBuffer(0) > 0.01)
   {
       /* startbuffer exists */
       tempElem = ReportXML::createXMLElem
           (doc, "EndBufferSize", QString::number(getEndBuffer(0)));
       taskElem.appendChild(tempElem);

       tempElem = ReportXML::createXMLElem
           (doc, "PlanEndBufferStart",
            QString::number(getEndBufferStart(0)));
       tempElem.setAttribute("humanReadable",
                             time2ISO(getEndBufferStart(0)));
       taskElem.appendChild(tempElem);

       tempElem = ReportXML::createXMLElem
           (doc, "PlanEndBufferStart",
            QString::number(getEndBufferStart(0)));
       tempElem.setAttribute("humanReadable",
                             time2ISO(getStartBufferEnd(0)));
       taskElem.appendChild(tempElem);
   }

   /* Responsible persons */
   if( getResponsible() )
      taskElem.appendChild( getResponsible()->xmlIDElement( doc ));

   /* Now start the subtasks */
   int cnt = 0;
   QDomElement subTaskElem = doc.createElement( "SubTasks" );
   for (Task* t = subFirst(); t != 0; t = subNext())
   {
      if( t != this )
      {
     QDomElement sTask = t->xmlElement( doc, false );
     subTaskElem.appendChild( sTask );
     cnt++;
      }
   }
   if( cnt > 0 )
      taskElem.appendChild( subTaskElem);

   /* list of tasks by id which are previous */
   if( previous.count() > 0 )
   {
      for (TaskListIterator tli(previous); *tli != 0; ++tli)
      {
     if( *tli != this )
     {
        taskElem.appendChild( ReportXML::createXMLElem( doc, "Previous",
                                                        (*tli)->getId()));
     }
      }
   }

   /* list of tasks by id which follow */
   if( followers.count() > 0 )
   {
      for (TaskListIterator tli(followers); *tli != 0; ++tli)
      {
     if( *tli != this )
     {
        taskElem.appendChild( ReportXML::createXMLElem( doc, "Follower",
                                                        (*tli)->getId()));
     }
      }
   }

   /** Allocations and Booked Resources
    *  With the following code, the task in XML contains simply a list of all Allocations
    *  wiht the ResourceID for which resource the allocation is. After that, there comes
    *  a list of all Resources, again having the Resource Id as key. That could be put
    *  in a hirarchy like
    *  <Resource Id="dev2" >Larry Bono
    *       <Income>1000</Income>
    *       <Allocation>
    *          <Load>100</Load>
    *          <Persistent>Yes</Persistent>
    *       </Allocation>
    *  </Resource>
    *
    *  But we do not ;-) to have full flexibility.
    *
    */
   /* Allocations */
   if( allocations.count() > 0 )
   {
      QPtrList<Allocation> al(allocations);
      for (QPtrListIterator<Allocation> ali(al); *ali != 0; ++ali)
      {
     taskElem.appendChild( (*ali)->xmlElement( doc ));
      }
   }

   /* booked Ressources */
   if( bookedResources.count() > 0 )
   {
       for (ResourceListIterator rli(bookedResources); *rli != 0; ++rli)
      {
     taskElem.appendChild( (*rli)->xmlIDElement( doc ));
      }
   }

   return( taskElem );
}

#ifdef HAVE_ICAL
#ifdef HAVE_KDE

void Task::toTodo( KCal::Todo* todo, KCal::CalendarLocal* /* cal */ )
{
   if( !todo ) return;
   QDateTime dt;

   // todo->setReadOnly( true );

   /* Start-Time of the task */
   dt.setTime_t( getPlanStart() );
   todo->setDtStart( dt );
   todo->setHasDueDate( true );

   /* Due-Time of the todo -> plan End  */
   dt.setTime_t( getPlanEnd());
   todo->setDtDue( dt );
   todo->setHasStartDate(true);

   /* Description and summary -> project ID */
   todo->setDescription( getNote() );
   todo->setSummary( getName() );
   todo->setPriority( getPriority() );
   todo->setCompleted( getComplete() );

   /* Resources */
   QPtrList<Resource> resList;
   resList = getPlanBookedResources();
   QStringList strList;

   Resource *res = 0;
   for (ResourceListIterator rli(resList); *rli != 0; ++rli)
   {
      strList.append( (*rli)->getName());
   }
   todo->setResources(strList);

}

#endif /* HAVE_KDE */
#endif /* HAVE_ICAL */

void Task::loadFromXML( QDomElement& parent, Project *project )
{
   QDomElement elem = parent.firstChild().toElement();

   for( ; !elem.isNull(); elem = elem.nextSibling().toElement() )
   {
      // qDebug(  "**Task -elemType: " + elem.tagName() );
      QString elemTagName = elem.tagName();

      if( elemTagName == "Name" )
      {
        setName( elem.text());
      }
      else if( elemTagName == "SubTasks" )
      {
     QDomElement subTaskElem = elem.firstChild().toElement();
     for( ; !subTaskElem.isNull(); subTaskElem = subTaskElem.nextSibling().toElement() )
     {
        /* Recursive call for more tasks */
        QString stId = subTaskElem.attribute("Id");
        qDebug( "Recursing to elem " + stId );
        Task *t = new Task( project, stId, QString(), this, QString(), 0 );
        t->inheritValues();
        t->loadFromXML( subTaskElem, project );
        qDebug( "Recursing to elem " + stId + " <FIN>");
     }
      }
      else if( elemTagName == "Type" )
      {
     if( elem.text() == "Milestone" )
        setMilestone();
     /* Container and Task are detected automatically */
      }
      else if( elemTagName == "Previous" )
      {
     addDepends( elem.text() );
      }
      else if( elemTagName == "Follower" )
      {
     addPrecedes( elem.text() );
      }
      else if( elemTagName == "Index" )
     setIndex( elem.text().toUInt());
      else if( elemTagName == "Priority" )
        setPriority( elem.text().toInt() );
      else if( elemTagName == "complete" )
     setComplete(0, elem.text().toInt() );

      /* time-stuff: */
      else if( elemTagName == "minStart" )
     setMinStart(0, elem.text().toLong());
      else if( elemTagName == "maxStart" )
     setMaxStart(0, elem.text().toLong() );
      else if( elemTagName == "minEnd" )
     setMinEnd(0, elem.text().toLong() );
      else if( elemTagName == "maxEnd" )
     setMaxEnd(0, elem.text().toLong() );
      else if( elemTagName == "actualStart" )
      {
          if (project->getScenarioIndex("actual") > 0)
              setStart(project->getScenarioIndex("actual"),
                       elem.text().toLong());
      }
      else if( elemTagName == "actualEnd" )
      {
          if (project->getScenarioIndex("actual") > 0)
              setEnd(project->getScenarioIndex("actual"),
                     elem.text().toLong());
      }
      else if( elemTagName == "planStart" )
     setStart(0, elem.text().toLong() );
      else if( elemTagName == "planEnd" )
     setEnd(0, elem.text().toLong() );

      /* Allocations */
      else if( elemTagName == "Allocation" )
      {
          allocationFromXML( elem );
      }
      else if( elemTagName == "Resource" )
      {
          const QString resId = elem.attribute( "Id" );
          Resource *r = project->getResource( resId );
          const QString name = elem.text();

          if( ! r )
          {
              r = new Resource( project, resId, name , 0L );
              r->inheritValues();
              project->addResource( r );
          }
          else
          {
              /* Resource does already exist, only correct the name */
              r->setName( name );
          }
      }
   }
}

void Task::allocationFromXML( const QDomElement& alloElem  )
{

    QString alloID = alloElem.attribute("ResourceID" );
    Project *project = getProject();
    if( ! project ) return;
    Resource *r = project->getResource( alloID );

    if( ! r )
    {
        /* Resource does not yet exist, create it. */
        r = new Resource( project, alloID, QString(), 0L );
        project->addResource( r );
    }

    Allocation *allocation = 0L;
    if( r )
    {
        allocation = new Allocation();
        allocation->addCandidate(r);
    }

    if( allocation )
    {
        QDomElement subElem = alloElem.firstChild().toElement();
        for( ; !subElem.isNull(); subElem = subElem.nextSibling().toElement() )
        {
            const QString tagName = subElem.tagName();

            if( tagName == "Load" )
            {
                UsageLimits* limits = new UsageLimits;
                limits->setDailyMax
                    ((uint) ((subElem.text().toDouble() *
                              project->getDailyWorkingHours() * 3600) /
                             project->getScheduleGranularity()));
                allocation->setLimits(limits);
            }
            else if( tagName == "Persistent" )
            {
                allocation->setPersistent( subElem.text() != "No" );
            }
        }
        addAllocation( allocation );
    }
}
