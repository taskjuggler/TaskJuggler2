/*
 * task.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

/* -- DTD --
 <!-- Task element, child of projects and for subtasks -->
 <!ELEMENT Task     (Index, Name, ProjectID, Priority, complete,
                     Type, ParentTask*, actualStart, actualEnd, planStart, planEnd,
                     SubTasks*, Previous*, Follower*, Allocation*,
                     Resource* )>
 <!ATTLIST Task         Id CDATA #REQUIRED>
 <!ELEMENT Index        (#PCDATA)>
 <!ELEMENT Name         (#PCDATA)>
 <!ELEMENT ProjectID    (#PCDATA)>
 <!ELEMENT Priority     (#PCDATA)>
 <!ELEMENT complete     (#PCDATA)>
 <!ELEMENT Type         (#PCDATA)>
 <!ELEMENT ParentTask   (#PCDATA)>
 <!ELEMENT Note         (#PCDATA)>
 <!ELEMENT Reference    (#PCDATA)>
 <!ELEMENT ReferenceLabel (#PCDATA)>
 <!ELEMENT minStart     (#PCDATA)>
 <!ELEMENT maxStart     (#PCDATA)>
 <!ELEMENT minEnd       (#PCDATA)>
 <!ELEMENT maxEnd       (#PCDATA)>
 <!ELEMENT actualStart  (#PCDATA)>
 <!ELEMENT actualEnd    (#PCDATA)>
 <!ELEMENT planStart    (#PCDATA)>
 <!ELEMENT planEnd      (#PCDATA)>
 <!ELEMENT startBufferSize (#PCDATA)>
 <!ELEMENT ActualStartBufferEnd (#PCDATA)>
 <!ELEMENT PlanStartBufferEnd   (#PCDATA)>
 <!ELEMENT endBufferSize        (#PCDATA)>
 <!ELEMENT ActualEndBufferStart (#PCDATA)>
 <!ELEMENT PlanEndBufferStart   (#PCDATA)>
 <!ELEMENT Resource             (#PCDATA)>
 <!ATTLIST Resource
           Id            CDATA #REQUIRED>
 <!ELEMENT SubTasks     (Task+)>
 <!ELEMENT Previous     (#PCDATA)>
 <!ELEMENT Follower     (#PCDATA)>
 <!ELEMENT bookedResources (ResourceID+)>

 <!-- Date values contain human readable date -->
 <!ATTLIST minStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST maxStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST minEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST maxEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST actualStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST actualEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST planStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST planEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST ActualStartBufferEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST PlanStartBufferEnd
           humanReadable CDATA #REQUIRED>
 <!ATTLIST ActualEndBufferStart
           humanReadable CDATA #REQUIRED>
 <!ATTLIST PlanEndBufferStart
           humanReadable CDATA #REQUIRED>
   /-- DTD --/
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
#include "CustomAttributeDefinition.h"

Task::Task(Project* proj, const QString& id_, const QString& n, Task* p,
           const QString& f, int l)
    : CoreAttributes(proj, id_, n, p), file(f), line(l)
{
    allocations.setAutoDelete(TRUE);
    shifts.setAutoDelete(TRUE);

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

        for (int sc = 0; sc < proj->getMaxScenarios(); ++sc)
        {
            scenarios[sc].minStart = p->scenarios[sc].minStart;
            scenarios[sc].maxStart = p->scenarios[sc].maxEnd;
            scenarios[sc].minEnd = p->scenarios[sc].minStart;
            scenarios[sc].maxEnd = p->scenarios[sc].maxEnd;
        }
        // Inherit depends from parent. Relative IDs need to get another '!'.
        dependsIds = p->dependsIds;
        for (QStringList::Iterator it = dependsIds.begin();
             it != dependsIds.end(); ++it)
        {
            if ((*it)[0] == '!')
                *it = '!' + *it;
        }

        // Inherit precedes from parent. Relative IDs need to get another '!'.
        precedesIds = p->precedesIds;
        for (QStringList::Iterator it = precedesIds.begin();
             it != precedesIds.end(); ++it)
        {
            if ((*it)[0] == '!')
                *it = '!' + *it;
        }

        // Inherit allocations from parent.
        for (QPtrListIterator<Allocation> ali(p->allocations); *ali; ++ali)
            allocations.append(new Allocation(**ali));

        // Inherit inheritable custom attributes
        inheritCustomAttributes(proj->getTaskAttributeDict());
    }
    else
    {
        // Set attributes that are inherited from global attributes.
        projectId = proj->getCurrentId();
        priority = proj->getPriority();
        for (int sc = 0; sc < proj->getMaxScenarios(); ++sc)
        {
            scenarios[sc].minStart = scenarios[sc].minEnd = 0;
            scenarios[sc].maxStart = scenarios[sc].maxEnd = 0;
        }
    }

    start = end = 0;
    duration = length = effort = 0.0;
}

Task::~Task()
{
    project->deleteTask(this);
    delete [] scenarios;
}

bool
Task::addDepends(const QString& rid)
{
    dependsIds.append(rid);
    return TRUE;
}

bool
Task::addPrecedes(const QString& rid)
{
    precedesIds.append(rid);
    return TRUE;
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

    TJMH.errorMessage(buf, file, line);
}

void
Task::schedule(time_t& date, time_t slotDuration)
{
    // Has the task been scheduled already or is it a container?
    if (schedulingDone || !sub.isEmpty())
        return;

    if (DEBUGTS(15))
        qDebug("Trying to schedule %s at %s",
               id.latin1(), time2tjp(date).latin1());

    if (scheduling == Task::ASAP)
    {
        if (start == 0)
            return;
        if (lastSlot == 0)
        {
            lastSlot = start - 1;
            doneEffort = 0.0;
            doneDuration = 0.0;
            doneLength = 0.0;
            workStarted = FALSE;
            tentativeEnd = date + slotDuration - 1;
            if (DEBUGTS(5))
                qDebug("Scheduling of %s starts at %s (%s)",
                       id.latin1(), time2tjp(lastSlot).latin1(),
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
        if (end == 0)
            return;
        if (lastSlot == 0)
        {
            lastSlot = end + 1;
            doneEffort = 0.0;
            doneDuration = 0.0;
            doneLength = 0.0;
            workStarted = FALSE;
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
            bookResources(date, slotDuration);

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
                propagateEnd();
            }
            else
            {
                start = date;
                propagateStart();
            }
            schedulingDone = TRUE;
            return;
        }
    }
    else if (effort > 0.0)
    {
        /* The effort of the task has been specified. We have to look
         * how much the resources can contribute over the following
         * workings days until we have reached the specified
         * effort. */
        bookResources(date, slotDuration);
        // Check whether we are done with this task.
        if (qRound(doneEffort * 2048) >= qRound(effort * 2048))
        {
            if (scheduling == ASAP)
            {
                end = tentativeEnd;
                propagateEnd();
            }
            else
            {
                start = tentativeStart;
                propagateStart();
            }
            schedulingDone = TRUE;
            return;
        }
    }
    else if (milestone)
    {
        // Task is a milestone.
        if (scheduling == ASAP)
        {
            end = start - 1;
            propagateEnd();
        }
        else
        {
            start = end + 1;
            propagateStart();
        }
        return;
    }
    else if (start != 0 && end != 0)
    {
        // Task with start and end date but no duration criteria.
        if (!allocations.isEmpty() && !project->isVacation(date))
            bookResources(date, slotDuration);

        if ((scheduling == ASAP && (date + slotDuration) >= end) ||
            (scheduling == ALAP && date <= start))
        {
            schedulingDone = TRUE;
            return;
        }
    }

    return;
}

bool
Task::scheduleContainer(bool safeMode)
{
    if (schedulingDone)
        return TRUE;

    time_t nstart = 0;
    time_t nend = 0;

    TaskListIterator tli(sub);
    // Check that this is really a container task
    if (*tli != 0)
    {
        if ((*tli)->start == 0 || (*tli)->end == 0)
            return TRUE;
        nstart = (*tli)->start;
        nend = (*tli)->end + ((*tli)->milestone ? 1 : 0);
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
        if ((*tli)->end + ((*tli)->milestone ? 1 : 0) > nend)
            nend = (*tli)->end + ((*tli)->milestone ? 1 : 0);
    }

    if (start == 0 || (!depends.isEmpty() && start < nstart))
    {
        start = nstart;
        propagateStart(safeMode);
    }

    if (end == 0 || (!precedes.isEmpty() && nend < end))
    {
        end = nend;
        propagateEnd(safeMode);
    }

    schedulingDone = TRUE;

    return FALSE;
}

void
Task::propagateStart(bool notUpwards)
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
            propagateEnd(notUpwards);
        }
    }

    /* Set start date to all previous that have no start date yet, but are
     * ALAP task or have no duration. */
    for (TaskListIterator tli(previous); *tli != 0; ++tli)
        if ((*tli)->end == 0 && (*tli)->latestEnd() != 0 &&
            ((*tli)->scheduling == ALAP ||
             ((*tli)->effort == 0.0 && (*tli)->length == 0.0 &&
              (*tli)->duration == 0.0 && !(*tli)->milestone)))
        {
            (*tli)->end = (*tli)->latestEnd();
            if (DEBUGTS(11))
                qDebug("PS2: Setting end of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->end).latin1());
            /* Recursively propagate the end date */
            (*tli)->propagateEnd(notUpwards);
        }

    /* Propagate start time to sub tasks which have only an implicit
     * dependancy on the parent task. Do not touch container tasks. */
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
    {
        if (!(*tli)->hasStartDependency())
        {
            (*tli)->start = start;
            if (DEBUGTS(11))
                qDebug("PS3: Setting start of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->start).latin1());
            /* Recursively propagate the start date */
            (*tli)->propagateStart(TRUE);
        }
    }

    if (notUpwards && parent)
    {
        if (DEBUGTS(11))
            qDebug("Scheduling parent of %s", id.latin1());
        getParent()->scheduleContainer(TRUE);
    }
}

void
Task::propagateEnd(bool notUpwards)
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
        schedulingDone = TRUE;
        if (start == 0)
        {
            start = end + 1;
            propagateStart(notUpwards);
        }
    }

    /* Set start date to all followers that have no start date yet, but are
     * ASAP task or have no duration. */
    for (TaskListIterator tli(followers); *tli != 0; ++tli)
        if ((*tli)->start == 0 && (*tli)->earliestStart() != 0 &&
            ((*tli)->scheduling == ASAP ||
             ((*tli)->effort == 0.0 && (*tli)->length == 0.0 &&
              (*tli)->duration == 0.0 && !(*tli)->milestone)))
        {
            (*tli)->start = (*tli)->earliestStart();
            if (DEBUGTS(11))
                qDebug("PE2: Setting start of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->start).latin1());
            /* Recursively propagate the start date */
            (*tli)->propagateStart(notUpwards);
        }
    /* Propagate end time to sub tasks which have only an implicit
     * dependancy on the parent task. Do not touch container tasks. */
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
        if (!(*tli)->hasEndDependency())
        {
            (*tli)->end = end;
            if (DEBUGTS(11))
                qDebug("PE3: Setting end of %s to %s",
                       (*tli)->id.latin1(), time2tjp((*tli)->end).latin1());
            /* Recursively propagate the end date */
            (*tli)->propagateEnd(TRUE);
        }

    if (notUpwards && parent)
    {
        if (DEBUGTS(11))
            qDebug("Scheduling parent of %s", id.latin1());
        getParent()->scheduleContainer(TRUE);
    }
}

void
Task::propagateInitialValues()
{
    if (start != 0)
        propagateStart(TRUE);
    if (end != 0)
        propagateEnd(TRUE);
    // Check if the some data of sub tasks can already be propagated.
    if (!sub.isEmpty())
        scheduleContainer(TRUE);
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
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
        if ((*tli)->isRunaway())
            return FALSE;

    return runAway;
}

void
Task::bookResources(time_t date, time_t slotDuration)
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
                     isAvailable(date, slotDuration, (*ali)->getLoad(),
                                 this)) > 0) 
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
                             (*rti)->isAvailable(date, slotDuration,
                                                 (*ali)->getLoad(), this)) > 0)
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
        /* If the allocation has be marked persistent and a resource
         * has already been picked, try to book this resource again. If the
         * resource is not available there will be no booking for this
         * time slot. */
        int maxAvailability = 0;
        if ((*ali)->isPersistent() && (*ali)->getLockedResource())
        {
            if (!bookResource((*ali)->getLockedResource(), date, slotDuration,
                         (*ali)->getLoad(), maxAvailability))
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
            QPtrList<Resource> cl = createCandidateList(date, *ali);
            bool found = FALSE;
            for (QPtrListIterator<Resource> rli(cl); *rli != 0; ++rli)
                if (bookResource((*rli), date, slotDuration,
                                 (*ali)->getLoad(), maxAvailability))
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
                   int loadFactor, int& maxAvailability)
{
    bool booked = FALSE;
    double intervalLoad = project->convertToDailyLoad(slotDuration);

    for (ResourceTreeIterator rti(r); *rti != 0; ++rti)
    {
        int availability;
        if ((availability = 
             (*rti)->isAvailable(date, slotDuration, loadFactor, this)) == 0)
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
Task::createCandidateList(time_t date, Allocation* a)
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
            while (candidates.getFirst())
            {
                cl.append(candidates.getFirst());
                candidates.remove(candidates.getFirst());
            }
            break;
        case Allocation::minLoaded:
        {
            while (!candidates.isEmpty())
            {
                double minLoad = 0;
                Resource* minLoaded = 0;
                for (QPtrListIterator<Resource> rli(candidates);
                     *rli != 0; ++rli)
                {
                    double load =
                        (*rli)->getCurrentLoad(Interval(project->getStart(),
                                                        date), 0) /
                        (*rli)->getMaxEffort();
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
            while (!candidates.isEmpty())
            {
                double maxLoad = 0;
                Resource* maxLoaded = 0;
                for (QPtrListIterator<Resource> rli(candidates);
                     *rli != 0; ++rli)
                {
                    double load =
                        (*rli)->getCurrentLoad(Interval(project->getStart(),
                                                        date), 0) /
                        (*rli)->getMaxEffort();
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
    if (scenarios[sc].complete != -1)
    {
        // some completion degree has been specified.
        return ((scenarios[sc].complete / 100.0) *
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
Task::earliestStart() const
{
    time_t date = 0;
    for (TaskListIterator tli(depends); *tli != 0; ++tli)
    {
        // All tasks this task depends on must have an end date set.
        if ((*tli)->end == 0)
            return 0;
        if ((*tli)->end > date)
            date = (*tli)->end;
    }
    if (date == 0)
        return 0;

    return date + 1;
}

time_t
Task::latestEnd() const
{
    time_t date = 0;
    for (TaskListIterator tli(precedes); *tli != 0; ++tli)
    {
        // All tasks this task precedes must have a start date set.
        if ((*tli)->start == 0)
            return 0;
        if (date == 0 || (*tli)->start < date)
            date = (*tli)->start;
    }
    if (date == 0)
        return 0;

    return date - 1;
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

    for (TaskListIterator tli(sub); *tli != 0; ++tli)
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

    if (recursive && !sub.isEmpty())
    {
        for (TaskListIterator tli(sub); *tli != 0; ++tli)
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

    if (DEBUGPF(2))
        qDebug("Creating cross references for task %s ...", id.latin1());

    for (QStringList::Iterator it = dependsIds.begin();
         it != dependsIds.end(); ++it)
    {
        QString absId = resolveId(*it);
        Task* t;
        if ((t = hash.find(absId)) == 0)
        {
            errorMessage(i18n("Unknown dependency '%1'").arg(absId));
            error = TRUE;
        }
        else if (depends.findRef(t) != -1)
        {
            errorMessage(i18n("No need to specify dependency %1 multiple "
                              "times.").arg(absId));
            // Make it a warning only for the time beeing.
            // error = TRUE;
        }
        else
        {
            depends.append(t);
            previous.append(t);
            t->successors.append(this);
            t->followers.append(this);
            if (DEBUGPF(11))
                qDebug("Registering follower %s with task %s",
                       id.latin1(), t->getId().latin1());
        }
    }

    for (QStringList::Iterator it = precedesIds.begin();
         it != precedesIds.end(); ++it)
    {
        QString absId = resolveId(*it);
        Task* t;
        if ((t = hash.find(absId)) == 0)
        {
            errorMessage(i18n("Unknown dependency '%1'").arg(absId));
            error = TRUE;
        }
        else if (precedes.findRef(t) != -1)
        {
            errorMessage(i18n("No need to specify dependency '%1'")
                         .arg(absId));
            // Make it a warning only for the time beeing.
            // error = TRUE;
        }
        else
        {
            precedes.append(t);
            followers.append(t);
            t->predecessors.append(this);
            t->previous.append(this);
            if (DEBUGPF(11))
                qDebug("Registering predecessor %s with task %s",
                       id.latin1(), t->getId().latin1());
        }
    }

    return !error;
}

void
Task::implicitXRef()
{
    for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
    {
        /* Propagate implicit dependencies. If a task has no specified start or
         * end date and no start or end dependencies, we check if a parent task
         * has an explicit start or end date which can be used. */
        if (!sub.isEmpty() || milestone)
            return;

        bool hasDurationSpec = scenarios[sc].duration != 0 ||
            scenarios[sc].length != 0 ||
            scenarios[sc].effort != 0;
        
        if (scenarios[sc].start == 0 && depends.isEmpty() &&
            !(hasDurationSpec && scheduling == ALAP))
            for (Task* tp = getParent(); tp; tp = tp->getParent())
            {
                if (tp->scenarios[sc].start != 0)
                {
                    if (DEBUGPF(11))
                        qDebug("Setting start of task '%s' to %s", id.latin1(),
                               time2ISO(tp->scenarios[sc].start).latin1());
                    scenarios[sc].start = tp->scenarios[sc].start;
                    break;
                }
            }
        /* And the same for end values */
        if (scenarios[sc].end == 0 && precedes.isEmpty() &&
            !(hasDurationSpec && scheduling == ASAP))
            for (Task* tp = getParent(); tp; tp = tp->getParent())
            {
                if (tp->scenarios[sc].end != 0)
                {
                    if (DEBUGPF(11))
                        qDebug("Setting end of task '%s' to %s", id.latin1(),
                               time2ISO(tp->scenarios[sc].end).latin1());
                    scenarios[sc].end = tp->scenarios[sc].end;
                    break;
                }
            }
    }
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
            for (TaskListIterator tli(sub); *tli != 0; ++tli)
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
        if (scheduling == ASAP && sub.isEmpty())
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
            for (TaskListIterator tli(sub); *tli != 0; ++tli)
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
        if (scheduling == ALAP && sub.isEmpty())
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
    if (scenarios[sc].start != 0 || !depends.isEmpty())
        return TRUE;
    for (Task* p = getParent(); p; p = p->getParent())
        if (p->scenarios[sc].start != 0)
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
    if (scenarios[sc].end != 0 || !precedes.isEmpty())
        return TRUE;
    for (Task* p = getParent(); p; p = p->getParent())
        if (p->scenarios[sc].end != 0)
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
    
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
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
    
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
        if ((*tli)->hasStartDependency())
            return TRUE;

    return FALSE;
}

bool
Task::preScheduleOk()
{
    for (int sc = 0; sc < project->getMaxScenarios(); sc++)
    {
        if (scenarios[sc].effort > 0.0 && allocations.count() == 0)
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
        if (!sub.isEmpty())
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
                             ("Milestone '%1' may not have a plan duration "
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
            if (scenarios[sc].start != 0 && scenarios[sc].end != 0 &&
                scenarios[sc].start != scenarios[sc].end + 1)
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
            if (((scenarios[sc].start != 0 && scenarios[sc].end != 0) ||
                 (hasStartDependency(sc) && scenarios[sc].start == 0 &&
                  scenarios[sc].end != 0 && scheduling == ASAP) ||
                 (scenarios[sc].start != 0 && scheduling == ALAP &&
                  hasEndDependency(sc) && scenarios[sc].end == 0)) &&
                durationSpec != 0)
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
                              "but no plan duration for the '%2' scenario.")
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
                          "but no account assigned.").arg(id));
            return FALSE;
        }

    }
    double intervalLoad =
        project->convertToDailyLoad(project->getScheduleGranularity());

    for (QPtrListIterator<Allocation> ali(allocations); *ali != 0; ++ali)
        if ((*ali)->getLoad() != 0 &&
            (*ali)->getLoad() < intervalLoad * 100.0)
        {
            QPtrListIterator<Resource> rli((*ali)->getCandidatesIterator());
            errorMessage(i18n
                         ("Warning: Load is smaller than scheduling "
                          "granularity (Task: '%1', Resource: '%2'). Minimal "
                          "load is '%3'.")
                         .arg(id).arg((*rli)->getId())
                         .arg(QString().sprintf("%.2f", intervalLoad +
                                                0.005)));
            (*ali)->setLoad((int) (intervalLoad * 100.0));
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
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
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
    for (TaskListIterator tli(depends); *tli != 0; ++tli)
        if ((*tli)->runAway)
            return FALSE;
    for (TaskListIterator tli(precedes); *tli != 0; ++tli)
        if ((*tli)->runAway)
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
        errorMessage(i18n("Start time of task '%1' is outside of the "
                          "project interval (%2 - %3) in '%4' scenario.")
                     .arg(time2tjp(start))
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
    if (scenarios[sc].maxStart != 0 && scenarios[sc].maxStart < start)
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
    if (end + (milestone ? 1 : 0) < project->getStart() ||
        end + (milestone ? 1 : 0) > project->getEnd())
    {
        errorMessage(i18n("End time of task '%1' is outside of the "
                          "project interval (%2 - %3) in '%4' scenario.")
                     .arg(time2tjp(end))
                     .arg(time2tjp(project->getStart()))
                     .arg(time2tjp(project->getEnd()))
                     .arg(scenario));
        errors++;
        return FALSE;
    }
    if (scenarios[sc].minEnd != 0 && 
        (end + (milestone ? 1 : 0) < scenarios[sc].minEnd))
    {
        errorMessage(i18n("'%1' end time of task '%2' is too early\n"
                          "Date is:  %3\n"
                          "Limit is: %4")
                     .arg(scenario).arg(id)
                     .arg(time2tjp(end + (milestone ? 1 : 0)))
                     .arg(time2tjp(scenarios[sc].minEnd)));
        errors++;
        return FALSE;
    }
    if (scenarios[sc].maxEnd != 0 && 
        (scenarios[sc].maxEnd < end + (milestone ? 1 : 0)))
    {
        errorMessage(i18n("'%1' end time of task '%2' is too late\n"
                          "Date is:  %2\n"
                          "Limit is: %3")
                     .arg(scenario).arg(id)
                     .arg(time2tjp(end + (milestone ? 1 : 0)))
                     .arg(time2tjp(scenarios[sc].maxEnd)));
        errors++;
        return FALSE;
    }
    if (!sub.isEmpty())
    {
        // All sub task must fit into their parent task.
        for (TaskListIterator tli(sub); *tli != 0; ++tli)
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
    if (schedulingDone || !sub.isEmpty())
        return 0;

    if (scheduling == ASAP && start != 0)
    {
        if (effort == 0.0 && length == 0.0 && duration == 0.0 && !milestone &&
            end == 0)
            return 0;

        if (lastSlot == 0)
            return start;
        return lastSlot + 1;
    }
    if (scheduling == ALAP && end != 0)
    {
        if (effort == 0.0 && length == 0.0 && duration == 0.0 && !milestone &&
            start == 0)
            return 0;
        if (lastSlot == 0)
            return end - slotDuration + 1;
        return lastSlot - slotDuration;
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
    for (TaskListIterator tli(sub); *tli != 0; ++tli)
        if (*tli == tsk || (*tli)->isSubTask(tsk))
            return TRUE;

    return FALSE;
}

void
Task::overlayScenario(int base, int sc)
{
    /* Copy all values that the scenario sc does not provide, but that are
     * provided by the base scenario to the scenario sc. */
    if (scenarios[sc].start == 0.0)
        scenarios[sc].start = scenarios[base].start;
    if (scenarios[sc].end == 0.0)
        scenarios[sc].end = scenarios[base].end;
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
    if (scenarios[sc].complete == -1)
        scenarios[sc].complete = scenarios[base].complete;
}

bool
Task::hasExtraValues(int sc) const
{
    return scenarios[sc].start != 0 || scenarios[sc].end != 0 ||
        scenarios[sc].length != 0 || scenarios[sc].duration != 0 ||
        scenarios[sc].effort != 0 || scenarios[sc].complete != -1 ||
        scenarios[sc].startBuffer >= 0.0 || scenarios[sc].endBuffer >= 0.0 ||
        scenarios[sc].startCredit >= 0.0 || scenarios[sc].endCredit >= 0.0;
}

void
Task::prepareScenario(int sc)
{
    start = scenarios[sc].start;
    end = scenarios[sc].end;

    duration = scenarios[sc].duration;
    length = scenarios[sc].length;
    effort = scenarios[sc].effort;
    lastSlot = 0;
    schedulingDone = scenarios[sc].scheduled;
    runAway = FALSE;
    bookedResources.clear();
    bookedResources = scenarios[sc].bookedResources;
    
    for (QPtrListIterator<Allocation> ali(allocations); *ali != 0; ++ali)
        (*ali)->init();
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
    if(scenarios[sc].complete != -1)
        return(scenarios[sc].complete);

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
    for (TaskListIterator tli(depends); *tli != 0; ++tli)
        if ((*tli)->parent == p)
            return TRUE;
    return FALSE;
}

bool
Task::precedesABrother(const Task* p) const
{
    for (TaskListIterator tli(precedes); *tli != 0; ++tli)
        if ((*tli)->parent == p)
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
                                             QString::number(scenarios[1].end +
                                                             (milestone ? 1 : 0)));
        tempElem.setAttribute( "humanReadable",
                               time2ISO(scenarios[1].end + (milestone ? 1 : 0)));
        taskElem.appendChild( tempElem );
    }

   tempElem = ReportXML::createXMLElem( doc, "planStart", QString::number( scenarios[0].start ));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[0].start ));
   taskElem.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "planEnd",
                                        QString::number(scenarios[0].end +
                                                        (milestone ? 1 : 0)));
   tempElem.setAttribute( "humanReadable", time2ISO( scenarios[0].end +
                                                     (milestone ? 1 : 0)));
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
        allocation = new Allocation( r );
    }

    if( allocation )
    {
        QDomElement subElem = alloElem.firstChild().toElement();
        for( ; !subElem.isNull(); subElem = subElem.nextSibling().toElement() )
        {
            const QString tagName = subElem.tagName();

            if( tagName == "Load" )
            {
                allocation->setLoad( subElem.text().toInt());
            }
            else if( tagName == "Persistent" )
            {
                allocation->setPersistent( subElem.text() != "No" );
            }
        }
        addAllocation( allocation );
    }
}
