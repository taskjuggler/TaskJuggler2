/*
 * Resource.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Resource.h"

#include <assert.h>

#include "ResourceTreeIterator.h"

#include "Project.h"
#include "Interval.h"
#include "VacationInterval.h"
#include "ShiftSelection.h"
#include "Booking.h"
#include "BookingList.h"
#include "SbBooking.h"
#include "Account.h"
#include "Task.h"
#include "UsageLimits.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "kotrus.h"
#include "debug.h"
#include "ReportXML.h"
#include "CustomAttributeDefinition.h"

/*
 * Calls to sbIndex are fairly expensive due to the floating point
 * division. We therefor use buffers that stores the index of the
 * first/last slot of a day/week/month for each slot.
 */
static uint* DayStartIndex = 0;
static uint* WeekStartIndex = 0;
static uint* MonthStartIndex = 0;
static uint* DayEndIndex = 0;
static uint* WeekEndIndex = 0;
static uint* MonthEndIndex = 0;


Resource::Resource(Project* p, const QString& i, const QString& n,
                   Resource* pr, const QString& df, uint dl) :
    CoreAttributes(p, i, n, pr, df, dl),
    minEffort(0.0),
    journal(),
    limits(0),
    efficiency(0.0),
    rate(0.0),
    kotrusId(),
    workingHours(),
    shifts(),
    vacations(),
    scoreboard(0),
    sbSize((p->getEnd() + 1 - p->getStart()) / p->getScheduleGranularity() + 1),
    specifiedBookings(new SbBooking**[p->getMaxScenarios()]),
    scoreboards(new SbBooking**[p->getMaxScenarios()]),
    scenarios(new ResourceScenario[p->getMaxScenarios()]),
    allocationProbability(new double[p->getMaxScenarios()])
{
    vacations.setAutoDelete(TRUE);
    shifts.setAutoDelete(TRUE);

    p->addResource(this);

    for (int sc = 0; sc < p->getMaxScenarios(); sc++)
    {
        scoreboards[sc] = 0;
        specifiedBookings[sc] = 0;
    }

    for (int i = 0; i < p->getMaxScenarios(); ++i)
        allocationProbability[i] = 0;

    if (!DayStartIndex)
    {
        DayStartIndex = new uint[sbSize];
        WeekStartIndex = new uint[sbSize];
        MonthStartIndex = new uint[sbSize];
        long i = 0;
        uint dayStart = 0;
        uint weekStart = 0;
        uint monthStart = 0;
        bool weekStartsMonday = project->getWeekStartsMonday();
        for (time_t ts = p->getStart(); i < (long) sbSize; ts +=
             p->getScheduleGranularity(), i++)
        {
            if (ts == midnight(ts))
                dayStart = i;
            DayStartIndex[i] = dayStart;

            if (ts == beginOfWeek(ts, weekStartsMonday))
                weekStart = i;
            WeekStartIndex[i] = weekStart;

            if (ts == beginOfMonth(ts))
                monthStart = i;
            MonthStartIndex[i] = monthStart;
        }

        DayEndIndex = new uint[sbSize];
        WeekEndIndex = new uint[sbSize];
        MonthEndIndex = new uint[sbSize];
        i = sbSize - 1;
        uint dayEnd = i;
        uint weekEnd = i;
        uint monthEnd = i;
        // WTF does p->getEnd not return the 1st sec after the time frame!!!
        for (time_t ts = p->getEnd() + 1; i >= 0;
             ts -= p->getScheduleGranularity(), i--)
        {
            DayEndIndex[i] = dayEnd;
            if (ts - midnight(ts) < (int) p->getScheduleGranularity())
                dayEnd = i > 0 ? i - 1 : 0;

            WeekEndIndex[i] = weekEnd;
            if (ts - beginOfWeek(ts, weekStartsMonday) <
                (int) p->getScheduleGranularity())
                weekEnd = i > 0 ? i - 1 : 0;

            MonthEndIndex[i] = monthEnd;
            if (ts - beginOfMonth(ts) < (int) p->getScheduleGranularity())
                monthEnd = i > 0 ? i - 1 : 0;
        }
    }

    for (int i = 0; i < 7; i++)
    {
        workingHours[i] = new QPtrList<Interval>();
        workingHours[i]->setAutoDelete(TRUE);
    }
}

Resource::~Resource()
{
    int i;
    for (i = 0; i < 7; i++)
        delete workingHours[i];

    for (int sc = 0; sc < project->getMaxScenarios(); sc++)
    {
        if (scoreboards[sc])
        {
            for (uint i = 0; i < sbSize; i++)
                if (scoreboards[sc][i] > (SbBooking*) 3)
                {
                    uint j;
                    for (j = i + 1; j < sbSize &&
                         scoreboards[sc][i] == scoreboards[sc][j]; j++)
                        ;
                    delete scoreboards[sc][i];
                    i = j - 1;
                }
            delete [] scoreboards[sc];
            scoreboards[sc] = 0;
        }
        if (specifiedBookings[sc])
        {
            for (uint i = 0; i < sbSize; i++)
                if (specifiedBookings[sc][i] > (SbBooking*) 3)
                {
                    uint j;
                    for (j = i + 1; j < sbSize &&
                         specifiedBookings[sc][i] == specifiedBookings[sc][j];
                         j++)
                        ;
                    delete specifiedBookings[sc][i];
                    i = j - 1;
                }
            delete [] specifiedBookings[sc];
            specifiedBookings[sc] = 0;
        }
    }
    delete [] allocationProbability;
    delete [] specifiedBookings;
    delete [] scoreboards;
    delete [] scenarios;

    delete limits;

    project->deleteResource(this);
}

void
Resource::deleteStaticData()
{
    delete [] DayStartIndex;
    delete [] WeekStartIndex;
    delete [] MonthStartIndex;
    delete [] DayEndIndex;
    delete [] WeekEndIndex;
    delete [] MonthEndIndex;
    DayStartIndex = 0;
    WeekStartIndex = 0;
    MonthStartIndex = 0;
    DayEndIndex = 0;
    WeekEndIndex = 0;
    MonthEndIndex = 0;
}

void
Resource::inheritValues()
{
    Resource* pr = (Resource*) parent;

    if (pr)
    {
        // Inherit flags from parent resource.
        for (QStringList::Iterator it = pr->flags.begin();
             it != pr->flags.end(); ++it)
            addFlag(*it);

        // Inherit default working hours from parent resource.
        for (int i = 0; i < 7; i++)
        {
            delete workingHours[i];
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(TRUE);
            for (QPtrListIterator<Interval> ivi(*pr->workingHours[i]);
                 *ivi != 0; ++ivi)
                workingHours[i]->append(new Interval(**ivi));
        }

        // Inherit vacation intervals from parent resource.
        for (QPtrListIterator<Interval> vli(pr->vacations);
             *vli != 0; ++vli)
            vacations.append(new Interval(**vli));

        minEffort = pr->minEffort;

        if (pr->limits)
            limits = new UsageLimits(*pr->limits);
        else
            limits = 0;

        rate = pr->rate;
        efficiency = pr->efficiency;

        // Inherit inheritable custom attributes
        inheritCustomAttributes(project->getResourceAttributeDict());
    }
    else
    {
        // Inherit from default working hours project defaults.
        for (int i = 0; i < 7; i++)
        {
            delete workingHours[i];
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(TRUE);
            for (QPtrListIterator<Interval>
                 ivi(project->getWorkingHoursIterator(i));
                 *ivi != 0; ++ivi)
                workingHours[i]->append(new Interval(**ivi));
        }

        minEffort = project->getMinEffort();

        if (project->getResourceLimits())
            limits = new UsageLimits(*project->getResourceLimits());
        else
            limits = 0;

        rate = project->getRate();
        efficiency = 1.0;
    }
}

void
Resource::setLimits(UsageLimits* l)
{
    if (limits)
        delete limits;
    limits = l;
}

void
Resource::initScoreboard()
{
    scoreboard = new SbBooking*[sbSize];

    // First mark all scoreboard slots as unavailable (1).
    for (uint i = 0; i < sbSize; i++)
        scoreboard[i] = (SbBooking*) 1;

    // Then change all worktime slots to 0 (available) again.
    for (time_t day = project->getStart(); day < project->getEnd() + 1;
         day += project->getScheduleGranularity())
    {
        if (isOnShift(Interval(day,
                               day + project->getScheduleGranularity() - 1)))
            scoreboard[sbIndex(day)] = (SbBooking*) 0;
    }

    // Then mark all resource specific vacation slots as such (2).
    for (QPtrListIterator<Interval> ivi(vacations); *ivi != 0; ++ivi)
        for (time_t date = (*ivi)->getStart() > project->getStart() ?
             (*ivi)->getStart() : project->getStart();
             date < (*ivi)->getEnd() && date < project->getEnd() + 1;
             date += project->getScheduleGranularity())
            scoreboard[sbIndex(date)] = (SbBooking*) 2;

    // Mark all global vacation slots as such (2)
    for (VacationList::Iterator ivi(project->getVacationListIterator());
         *ivi != 0; ++ivi)
    {
        if ((*ivi)->getStart() > project->getEnd() ||
            (*ivi)->getEnd() < project->getStart())
            continue;
        uint startIdx = sbIndex((*ivi)->getStart() >= project->getStart() ?
                                (*ivi)->getStart() : project->getStart());
        uint endIdx = sbIndex((*ivi)->getEnd() >= project->getStart() ?
                              (*ivi)->getEnd() : project->getEnd());
        for (uint idx = startIdx; idx <= endIdx; ++idx)
            scoreboard[idx] = (SbBooking*) 2;
    }
}

uint
Resource::sbIndex(time_t date) const
{
    assert(date >= project->getStart());
    assert(date <= project->getEnd());
    // Convert date to corresponding scoreboard index.
    uint sbIdx = (date - project->getStart()) /
        project->getScheduleGranularity();
    assert(sbIdx < sbSize);
    return sbIdx;
}

time_t
Resource::index2start(uint idx) const
{
    return project->getStart() + idx *
        project->getScheduleGranularity();
}

time_t
Resource::index2end(uint idx) const
{
    return project->getStart() + (idx + 1) *
        project->getScheduleGranularity() - 1;
}

/**
 * \retval 0 { resource is available }
 * \retval 1 { resource is on vacation }
 * \retval 2 { resource is generally overloaded }
 * \retval 3 { resource is overloaded for this task }
 * \retval 4 { resource is allocated to other task }
 */
int
Resource::isAvailable(time_t date)
{
    /* The scoreboard of a resource is only generated on demand, so that large
     * resource lists that are only scarcely used for the project do not slow
     * TJ down too much. */
    if (!scoreboard)
        initScoreboard();
    // Check if the interval is booked or blocked already.
    uint sbIdx = sbIndex(date);
    if (scoreboard[sbIdx])
    {
        if (DEBUGRS(6))
        {
            QString reason;
            if (scoreboard[sbIdx] == ((SbBooking*) 1))
                reason = "off-hour";
            else if (scoreboard[sbIdx] == ((SbBooking*) 2))
                reason = "vacation";
            else if (scoreboard[sbIdx] == ((SbBooking*) 3))
                reason = "UNDEFINED";
            else
                reason = "allocated to " +
                    scoreboard[sbIdx]->getTask()->getId();
            qDebug("  Resource %s is busy (%s)", id.latin1(),
                   reason.latin1());
        }
        return scoreboard[sbIdx] < ((SbBooking*) 4) ? 1 : 4;
    }

    if (!limits)
        return 0;

    if ((limits && limits->getDailyMax() > 0))
    {
        // Now check that the resource is not overloaded on this day.
        uint bookedSlots = 1;

        for (uint i = DayStartIndex[sbIdx]; i <= DayEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (b < (SbBooking*) 4)
                continue;

            bookedSlots++;
        }

        if (limits && limits->getDailyMax() > 0 &&
            bookedSlots > limits->getDailyMax())
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded today (%d)", id.latin1(),
                       bookedSlots);
            return 2;
        }
    }
    if ((limits && limits->getWeeklyMax() > 0))
    {
        // Now check that the resource is not overloaded on this week.
        uint bookedSlots = 1;

        for (uint i = WeekStartIndex[sbIdx]; i <= WeekEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (b < (SbBooking*) 4)
                continue;

            bookedSlots++;
        }

        if (limits && limits->getWeeklyMax() > 0 &&
            bookedSlots > limits->getWeeklyMax())
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded this week (%d)", id.latin1(),
                       bookedSlots);
            return 2;
        }
    }
    if ((limits && limits->getMonthlyMax() > 0))
    {
        // Now check that the resource is not overloaded on this month.
        uint bookedSlots = 1;

        for (uint i = MonthStartIndex[sbIdx]; i <= MonthEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (b < (SbBooking*) 4)
                continue;

            bookedSlots++;
        }

        if (limits && limits->getMonthlyMax() > 0 &&
            bookedSlots > limits->getMonthlyMax())
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded this month (%d)", id.latin1(),
                       bookedSlots);
            return 2;
        }
    }

    return 0;
}

bool
Resource::book(Booking* nb)
{
    uint idx = sbIndex(nb->getStart());

    return bookSlot(idx, nb, 0);
}

bool
Resource::bookSlot(uint idx, SbBooking* nb, int overtime)
{
    // Make sure that the time slot is still available.
    if (scoreboard[idx] > (SbBooking*) overtime)
    {
        delete nb;
        return FALSE;
    }

    SbBooking* b;
    // Try to merge the booking with the booking in the previous slot.
    if (idx > 0 && (b = scoreboard[idx - 1]) > (SbBooking*) 3 &&
        b->getTask() == nb->getTask())
    {
        scoreboard[idx] = b;
        delete nb;
        return TRUE;
    }
    // Try to merge the booking with the booking in the following slot.
    if (idx < sbSize - 1 && (b = scoreboard[idx + 1]) > (SbBooking*) 3 &&
        b->getTask() == nb->getTask())
    {
        scoreboard[idx] = b;
        delete nb;
        return TRUE;
    }
    scoreboard[idx] = nb;
    return TRUE;
}

bool
Resource::bookInterval(Booking* nb, int sc, int sloppy, int overtime)
{
    uint sIdx = sbIndex(nb->getStart());
    uint eIdx = sbIndex(nb->getEnd());

    bool conflict = FALSE;

    for (uint i = sIdx; i <= eIdx; i++)
        if (scoreboard[i] > (SbBooking*) overtime)
        {
            uint j;
            for (j = i + 1 ; j <= eIdx &&
                 scoreboard[i] == scoreboard[j]; j++)
                ;
            if (scoreboard[i] == (SbBooking*) 1)
            {
                if (sloppy > 0)
                {
                    i = j;
                    continue;
                }
                TJMH.errorMessage
                    (i18n("Error in %1 scenario: "
                          "%2 has no duty hours at %3 - %4 "
                          "to be assigned to %5.")
                     .arg(project->getScenarioId(sc))
                     .arg(id).arg(time2ISO(index2start(i)))
                     .arg(time2ISO(index2end(j  - 1)))
                     .arg(nb->getTask()->getId().latin1()));
            }
            else if (scoreboard[i] == (SbBooking*) 2)
            {
                if (sloppy > 1)
                {
                    i = j;
                    continue;
                }
                TJMH.errorMessage
                    (i18n("Error in %1 scenario: "
                          "%2 is on vacation at %3 - %4. "
                          "It cannot be assigned to %5.")
                     .arg(project->getScenarioId(sc))
                     .arg(id).arg(time2ISO(index2start(i)))
                     .arg(time2ISO(index2end(j - 1)))
                     .arg(nb->getTask()->getId().latin1()));
            }
            else
            {
                if (sloppy > 2)
                {
                    i = j;
                    continue;
                }
                TJMH.errorMessage
                    (i18n("Error in %1 scenario: "
                          "Allocation conflict for %2 at %3 - %4. "
                          "Conflicting tasks are %5 and %6.")
                     .arg(project->getScenarioId(sc))
                     .arg(id).arg(time2ISO(index2start(i)))
                     .arg(time2ISO(index2end(j - 1)))
                     .arg(scoreboard[i]->getTask()->getId().latin1())
                     .arg(nb->getTask()->getId().latin1()));
            }

            conflict = TRUE;
            i = j;
        }

    if (conflict)
        return FALSE;

    for (uint i = sIdx; i <= eIdx; i++)
        if (scoreboard[i] <= (SbBooking*) overtime)
            bookSlot(i, new SbBooking(*nb), overtime);

    return TRUE;
}

bool
Resource::addBooking(int sc, Booking* nb, int sloppy, int overtime)
{
    SbBooking** tmp = scoreboard;

    if (scoreboards[sc])
        scoreboard = scoreboards[sc];
    else
        initScoreboard();
    bool retVal = bookInterval(nb, sc, sloppy, overtime);
    // Cross register booking with task.
    if (retVal && nb->getTask())
        nb->getTask()->addBookedResource(sc, this);
    delete nb;
    scoreboards[sc] = scoreboard;
    scoreboard = tmp;
    return retVal;
}

bool
Resource::addShift(const Interval& i, Shift* s)
{
    return shifts.insert(new ShiftSelection(i, s));
}

bool
Resource::addShift(ShiftSelection* s)
{
    return shifts.insert(s);
}

void
Resource::addVacation(Interval* i)
{
    vacations.append(i);
}

bool
Resource::isWorker() const
{
    for (ConstResourceTreeIterator rti(this); *rti; ++rti)
        if ((*rti)->efficiency == 0.0)
            return false;

    return true;
}

double
Resource::getCurrentLoad(const Interval& period, const Task* task) const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return 0.0;

    return efficiency * project->convertToDailyLoad
        (getCurrentLoadSub(sbIndex(iv.getStart()), sbIndex(iv.getEnd()), task) *
         project->getScheduleGranularity());
}

long
Resource::getCurrentLoadSub(uint startIdx, uint endIdx, const Task* task) const
{
    long bookings = 0;

    for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
        bookings += (*rli)->getCurrentLoadSub(startIdx, endIdx, task);

    if (!scoreboard)
        return bookings;

    for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
    {
        SbBooking* b = scoreboard[i];
        if (b < (SbBooking*) 4)
            continue;
        if (task == 0 || task == b->getTask())
            bookings++;
    }

    return bookings;
}

uint
Resource::getCurrentDaySlots(time_t date, const Task* t)
{
    /* Return the number of slots this resource is allocated to in the current
     * scenario. If a task is given, only the slots allocated to this task
     * will be counted. */

    if (hasSubs())
    {
        uint timeSlots = 0;
        for (ResourceListIterator rli(getSubListIterator()); *rli; ++rli)
            timeSlots += (*rli)->getCurrentDaySlots(date, t);
        return timeSlots;
    }

    if (!scoreboard)
        return 0;

    uint sbIdx = sbIndex(date);

    // Now check that the resource is not overloaded on this day.
    uint bookedSlots = 0;
    uint bookedTaskSlots = 0;

    for (uint i = DayStartIndex[sbIdx]; i <= DayEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (b < (SbBooking*) 4)
            continue;

        bookedSlots++;
        if (b->getTask() == t)
            bookedTaskSlots++;
    }
    // If the current slot is still available we count this as booked as well.
    if (scoreboard[sbIdx] < ((SbBooking*) 4))
    {
        bookedSlots++;
        bookedTaskSlots++;
    }

    return t ? bookedTaskSlots : bookedSlots;
}

uint
Resource::getCurrentWeekSlots(time_t date, const Task* t)
{
    /* Return the number of slots this resource is allocated to in the current
     * scenario. If a task is given, only the slots allocated to this task
     * will be counted. */

    if (hasSubs())
    {
        uint timeSlots = 0;
        for (ResourceListIterator rli(getSubListIterator()); *rli; ++rli)
            timeSlots += (*rli)->getCurrentWeekSlots(date, t);
        return timeSlots;
    }

    if (!scoreboard)
        return 0;

    uint sbIdx = sbIndex(date);

    // Now check that the resource is not overloaded on this day.
    uint bookedSlots = 0;
    uint bookedTaskSlots = 0;

    for (uint i = WeekStartIndex[sbIdx]; i <= WeekEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (b < (SbBooking*) 4)
            continue;

        bookedSlots++;
        if (b->getTask() == t)
            bookedTaskSlots++;
    }

    // If the current slot is still available we count this as booked as well.
    if (scoreboard[sbIdx] < ((SbBooking*) 4))
    {
        bookedSlots++;
        bookedTaskSlots++;
    }

    return t ? bookedTaskSlots : bookedSlots;
}

uint
Resource::getCurrentMonthSlots(time_t date, const Task* t)
{
    /* Return the number of slots this resource is allocated to in the current
     * scenario. If a task is given, only the slots allocated to this task
     * will be counted. */

    if (hasSubs())
    {
        uint timeSlots = 0;
        for (ResourceListIterator rli(getSubListIterator()); *rli; ++rli)
            timeSlots += (*rli)->getCurrentMonthSlots(date, t);
        return timeSlots;
    }

    if (!scoreboard)
        return 0;

    uint sbIdx = sbIndex(date);

    // Now check that the resource is not overloaded on this day.
    uint bookedSlots = 0;
    uint bookedTaskSlots = 0;

    for (uint i = MonthStartIndex[sbIdx]; i <= MonthEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (b < (SbBooking*) 4)
            continue;

        bookedSlots++;
        if (b->getTask() == t)
            bookedTaskSlots++;
    }

    // If the current slot is still available we count this as booked as well.
    if (scoreboard[sbIdx] < ((SbBooking*) 4))
    {
        bookedSlots++;
        bookedTaskSlots++;
    }

    return t ? bookedTaskSlots : bookedSlots;
}

double
Resource::getLoad(int sc, const Interval& period, AccountType acctType,
                  const Task* task) const
{
    return efficiency * getAllocatedTimeLoad(sc, period, acctType, task);
}

double
Resource::getAllocatedTimeLoad(int sc, const Interval& period,
                               AccountType acctType, const Task* task) const
{
    return project->convertToDailyLoad
        (getAllocatedTime(sc, period, acctType, task));
}

long
Resource::getAllocatedTime(int sc, const Interval& period, AccountType acctType,
                          const Task* task) const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return 0;
    uint startIdx = sbIndex(iv.getStart());
    uint endIdx = sbIndex(iv.getEnd());
    if (scenarios[sc].firstSlot > 0 && scenarios[sc].lastSlot > 0)
    {
        if (startIdx < (uint) scenarios[sc].firstSlot)
            startIdx = scenarios[sc].firstSlot;
        if (endIdx > (uint) scenarios[sc].lastSlot)
            endIdx = scenarios[sc].lastSlot;
    }

    return getAllocatedSlots(sc, startIdx, endIdx, acctType, task) *
        project->getScheduleGranularity();
}

long
Resource::getAllocatedSlots(int sc, uint startIdx, uint endIdx,
                            AccountType acctType, const Task* task) const
{
    long bookings = 0;

    if (isGroup())
    {
        for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
            bookings += (*rli)->getAllocatedSlots(sc, startIdx, endIdx,
                                                  acctType, task);
        return bookings;
    }

    // If the scoreboard has not been initialized there is no load.
    if (!scoreboards[sc])
        return bookings;

    if (task && scenarios[sc].firstSlot >= 0 && scenarios[sc].lastSlot >= 0)
    {
        /* If the load is to be calculated for a certain task, we check
         * whether this task is in the resource allocation list. Only then we
         * calculate the real number of allocated slots. */
        bool isAllocated = FALSE;
        for (TaskListIterator tli(scenarios[sc].allocatedTasks); *tli; ++tli)
            if (task == *tli)
            {
                isAllocated = TRUE;
                break;
            }
        if (!isAllocated)
            return bookings;
    }

    for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (b < (SbBooking*) 4)
            continue;
        if ((task == 0 ||
             (task != 0 && task == b->getTask())) &&
            (acctType == AllAccounts ||
             (b->getTask()->getAccount() &&
              b->getTask()->getAccount()->getAcctType() == acctType)))
            bookings++;
    }

    return bookings;
}

double
Resource::getAvailableWorkLoad(int sc, const Interval& period)
{
    return efficiency * getAvailableTimeLoad(sc, period);
}

double
Resource::getAvailableTimeLoad(int sc, const Interval& period)
{
    return project->convertToDailyLoad(getAvailableTime(sc, period));
}

long
Resource::getAvailableTime(int sc, const Interval& period)
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return 0;

    return getAvailableSlots(sc, sbIndex(iv.getStart()),
                             sbIndex(iv.getEnd())) *
        project->getScheduleGranularity();
}

long
Resource::getAvailableSlots(int sc, uint startIdx, uint endIdx)
{
    long availSlots = 0;

    if (!sub->isEmpty())
    {
        for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
            availSlots += (*rli)->getAvailableSlots(sc, startIdx, endIdx);
    }
    else
    {
        if (!scoreboards[sc])
        {
            scoreboard = scoreboards[sc];
            initScoreboard();
            scoreboards[sc] = scoreboard;
        }

        for (uint i = startIdx; i <= endIdx; i++)
            if (scoreboards[sc][i] == 0)
                availSlots++;
    }

    return availSlots;
}

double
Resource::getCredits(int sc, const Interval& period, AccountType acctType,
                     const Task* task) const
{
    return project->convertToDailyLoad
        (getAllocatedTime(sc, period, acctType, task)) * rate;
}

bool
Resource::isAllocated(int sc, const Interval& period, const QString& prjId)
    const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return FALSE;

    uint startIdx = sbIndex(iv.getStart());
    uint endIdx = sbIndex(iv.getEnd());
    if (scenarios[sc].firstSlot > 0 && scenarios[sc].lastSlot > 0)
    {
        if (startIdx < (uint) scenarios[sc].firstSlot)
            startIdx = scenarios[sc].firstSlot;
        if (endIdx > (uint) scenarios[sc].lastSlot)
            endIdx = scenarios[sc].lastSlot;
    }

    if (endIdx < startIdx)
        return FALSE;

    return isAllocatedSub(sc, startIdx, endIdx, prjId);
}

bool
Resource::isAllocatedSub(int sc, uint startIdx, uint endIdx, const QString&
                         prjId) const
{
    /* If resource is a group, check members first. */
    for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
        if ((*rli)->isAllocatedSub(sc, startIdx, endIdx, prjId))
            return TRUE;

    if (!scoreboards[sc])
        return FALSE;
    for (uint i = startIdx; i <= endIdx; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (b < (SbBooking*) 4)
            continue;
        if (prjId.isNull() || b->getTask()->getProjectId() == prjId)
            return TRUE;
    }
    return FALSE;
}

bool
Resource::isAllocated(int sc, const Interval& period, const Task* task) const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return FALSE;

    uint startIdx = sbIndex(iv.getStart());
    uint endIdx = sbIndex(iv.getEnd());
    if (scenarios[sc].firstSlot > 0 && scenarios[sc].lastSlot > 0)
    {
        if (startIdx < (uint) scenarios[sc].firstSlot)
            startIdx = scenarios[sc].firstSlot;
        if (endIdx > (uint) scenarios[sc].lastSlot)
            endIdx = scenarios[sc].lastSlot;
    }
    if (endIdx < startIdx)
        return FALSE;

    return isAllocatedSub(sc, startIdx, endIdx, task);
}

bool
Resource::isAllocatedSub(int sc, uint startIdx, uint endIdx, const Task* task)
    const
{
    /* If resource is a group, check members first. */
    for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
        if ((*rli)->isAllocatedSub(sc, startIdx, endIdx, task))
            return TRUE;

    if (!scoreboards[sc])
        return FALSE;
    for (uint i = startIdx; i <= endIdx; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (b < (SbBooking*) 4)
            continue;
        if (task == 0 || b->getTask() == task)
            return TRUE;
    }
    return FALSE;
}

void
Resource::getPIDs(int sc, const Interval& period, const Task* task,
                  QStringList& pids) const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return;

    for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
        (*rli)->getPIDs(sc, iv, task, pids);

    if (!scoreboards[sc])
        return;
    for (uint i = sbIndex(iv.getStart());
         i <= sbIndex(iv.getEnd()) && i < sbSize; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (b < (SbBooking*) 4)
            continue;
        if ((task == 0 || task == b->getTask()) &&
            pids.findIndex(b->getTask()->getProjectId()) == -1)
        {
            pids.append(b->getTask()->getProjectId());
        }
    }
}

QString
Resource::getProjectIDs(int sc, const Interval& period, const Task* task) const
{
    QStringList pids;
    getPIDs(sc, period, task, pids);
    QString pidStr;
    for (QStringList::Iterator it = pids.begin(); it != pids.end(); ++it)
        pidStr += QString(it != pids.begin() ? ", " : "") + *it;

    return pidStr;
}

bool
Resource::dbLoadBookings(const QString& kotrusID,
                         const QStringList& skipProjectIDs)
{
    /* retrieve all bookings _not_ belonging to this project */
    BookingList blist = project->getKotrus()->loadBookings
        (kotrusID, skipProjectIDs);
    return TRUE;
}

bool
Resource::hasVacationDay(time_t day) const
{
    Interval fullDay(midnight(day),
                     sameTimeNextDay(midnight(day)) - 1);
    for (QPtrListIterator<Interval> vli(vacations); *vli != 0; ++vli)
        if ((*vli)->overlaps(fullDay))
            return TRUE;

    if (shifts.isVacationDay(day))
        return TRUE;

    if (workingHours[dayOfWeek(day, FALSE)]->isEmpty())
        return TRUE;

    return FALSE;
}

bool
Resource::isOnShift(const Interval& slot) const
{
    for (ShiftSelectionList::Iterator ssli(shifts); *ssli != 0; ++ssli)
        if ((*ssli)->getPeriod().contains(slot))
            return (*ssli)->getShift()->isOnShift(slot);

    int dow = dayOfWeek(slot.getStart(), FALSE);
    for (QPtrListIterator<Interval> ivi(*workingHours[dow]); *ivi != 0; ++ivi)
        if ((*ivi)->contains(Interval(secondsOfDay(slot.getStart()),
                                      secondsOfDay(slot.getEnd()))))
            return TRUE;

    return FALSE;
}

void
Resource::setWorkingHours(int day, const QPtrList<Interval>& l)
{
    delete workingHours[day];

    // Create a deep copy of the interval list.
    workingHours[day] = new QPtrList<Interval>;
    workingHours[day]->setAutoDelete(true);
    for (QPtrListIterator<Interval> pli(l); pli; ++pli)
        workingHours[day]->append(new Interval(**pli));
}

BookingList
Resource::getJobs(int sc) const
{
    BookingList bl;
    if (scoreboards[sc])
    {
        SbBooking* b = 0;
        uint startIdx = 0;
        for (uint i = 0; i < sbSize; i++)
            if (scoreboards[sc][i] != b)
            {
                if (b)
                    bl.append(new Booking(Interval(index2start(startIdx),
                                                   index2end(i - 1)),
                                          scoreboards[sc][startIdx]));
                if (scoreboards[sc][i] > (SbBooking*) 3)
                {
                    b = scoreboards[sc][i];
                    startIdx = i;
                }
                else
                    b = 0;
            }
    }
    return bl;
}

time_t
Resource::getStartOfFirstSlot(int sc, const Task* task)
{
    if (scoreboards[sc] == 0)
        return 0;
    for (uint i = 0; i < sbSize; ++i)
    {
        if (scoreboards[sc][i] > ((SbBooking*) 3) &&
            scoreboards[sc][i]->getTask() == task)
            return index2start(i);
    }

    return 0;
}

time_t
Resource::getEndOfLastSlot(int sc, const Task* task)
{
    if (scoreboards[sc] == 0)
        return 0;
    int i = sbSize;
    for ( ; ; )
    {
        --i;
        if (scoreboards[sc][i] > ((SbBooking*) 3) &&
            scoreboards[sc][i]->getTask() == task)
            return index2end(i);
        if (i == 0)
            break;
    }

    return 0;
}

void
Resource::copyBookings(int sc, SbBooking*** src, SbBooking*** dst)
{
    /* This function copies a set of bookings the specified scenario. If the
     * destination set already contains bookings it is cleared first.
     */
    if (dst[sc])
        for (uint i = 0; i < sbSize; i++)
            if (dst[sc][i] > (SbBooking*) 3)
            {
                /* Small pointers are fake bookings. We can safely ignore
                 * them. Identical pointers in successiv slots must only be
                 * deleted once. */
                uint j;
                for (j = i + 1; j < sbSize &&
                     dst[sc][i] == dst[sc][j]; j++)
                    ;
                delete dst[sc][i];
                i = j - 1;
            }

    // Now copy the source set to the destination.
    if (src[sc])
    {
        if (!dst[sc])
            dst[sc] = new SbBooking*[sbSize];
        for (uint i = 0; i < sbSize; i++)
            if (src[sc][i] > (SbBooking*) 3)
            {
                /* Small pointers can just be copied. Identical successiv
                 * pointers need to be allocated once and can then be assigned
                 * to all destination slots. */
                dst[sc][i] = new SbBooking(src[sc][i]);
                uint j;
                for (j = i + 1; j < sbSize &&
                     src[sc][i] == src[sc][j]; j++)
                    dst[sc][j] = dst[sc][i];
                i = j - 1;
            }
            else
                dst[sc][i] = src[sc][i];
    }
    else
    {
        delete [] dst[sc];
        dst[sc] = 0;
    }
}

void
Resource::saveSpecifiedBookings()
{
    for (int sc = 0; sc < project->getMaxScenarios(); sc++)
        copyBookings(sc, scoreboards, specifiedBookings);
}

void
Resource::prepareScenario(int sc)
{
    copyBookings(sc, specifiedBookings, scoreboards);
    scoreboard = scoreboards[sc];
    scenarios[sc].allocatedTasks.clear();

    scenarios[sc].firstSlot = -1;
    scenarios[sc].lastSlot = -1;
}

void
Resource::finishScenario(int sc)
{
    scoreboards[sc] = scoreboard;

    if (!scoreboard)
        return;

    scenarios[sc].firstSlot = -1;
    scenarios[sc].lastSlot = -1;
    /* Create a list of all tasks that the resource is allocated to in this
     * scenario. */
    for (uint i = 0; i < sbSize; i++)
        if (scoreboard[i] > (SbBooking*) 4)
        {
            if (scenarios[sc].firstSlot == -1)
                scenarios[sc].firstSlot = i;
            scenarios[sc].lastSlot = i;
            scenarios[sc].addTask(scoreboard[i]->getTask());
        }
}

bool
Resource::bookingsOk(int sc)
{
    if (scoreboards[sc] == 0)
        return TRUE;

    if (hasSubs())
    {
       TJMH.errorMessage
          (i18n("Group resource '%1' may not have bookings") .arg(id));
       return FALSE;
    }

    for (uint i = 0; i < sbSize; ++i)
        if (scoreboards[sc][i] >= ((SbBooking*) 4))
        {
            time_t start = index2start(i);
            time_t end = index2end(i);
            time_t tStart = scoreboards[sc][i]->getTask()->getStart(sc);
            time_t tEnd = scoreboards[sc][i]->getTask()->getEnd(sc);
            if (start < tStart || start > tEnd ||
                end < tStart || end > tEnd)
            {
                TJMH.errorMessage
                    (i18n("Booking of resource '%1' on task '%2' at %3 "
                          "is outside of task interval (%4 - %5) "
                          "in scenario '%6'")
                     .arg(id).arg(scoreboards[sc][i]->getTask()->getId())
                     .arg(time2ISO(start)).arg(time2ISO(tStart))
                     .arg(time2ISO(tEnd)).arg(project->getScenarioId(sc)));
                return FALSE;
            }
        }

    return TRUE;
}

void
Resource::addJournalEntry(JournalEntry* entry)
{
    journal.append(entry);
}

Journal::Iterator
Resource::getJournalIterator() const
{
    return Journal::Iterator(journal);
}

QDomElement Resource::xmlIDElement( QDomDocument& doc ) const
{
   QDomElement elem = ReportXML::createXMLElem( doc, "Resource", getName());
   elem.setAttribute( "Id", getId() );

   return( elem );
}



