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
#include "ShiftSelection.h"
#include "BookingList.h"
#include "Account.h"
#include "UsageLimits.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "ReportXML.h"
#include "CustomAttributeDefinition.h"

/*
 * Calls to sbIndex are fairly expensive due to the floating point
 * division. We therefor use buffers that stores the index of the
 * first/last slot of a day/week/month/year for each slot.
 */
static uint* DayStartIndex = 0;
static uint* WeekStartIndex = 0;
static uint* MonthStartIndex = 0;
static uint* YearStartIndex = 0;
static uint* DayEndIndex = 0;
static uint* WeekEndIndex = 0;
static uint* MonthEndIndex = 0;
static uint* YearEndIndex = 0;


Resource::Resource(Project* p, const QString& i, const QString& n,
                   Resource* pr, const QString& df, uint dl) :
    CoreAttributes(p, i, n, pr, df, dl),
    minEffort(0.0),
    journal(),
    limits(0),
    efficiency(0.0),
    rate(0.0),
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
    vacations.setAutoDelete(true);
    shifts.setAutoDelete(true);

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
        YearStartIndex = new uint[sbSize];
        long i = 0;
        uint dayStart = 0;
        uint weekStart = 0;
        uint monthStart = 0;
        uint yearStart = 0;
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

            if (ts == beginOfYear(ts))
                yearStart = i;
            YearStartIndex[i] = yearStart;
        }

        DayEndIndex = new uint[sbSize];
        WeekEndIndex = new uint[sbSize];
        MonthEndIndex = new uint[sbSize];
        YearEndIndex = new uint[sbSize];
        i = sbSize - 1;
        uint dayEnd = i;
        uint weekEnd = i;
        uint monthEnd = i;
        uint yearEnd = i;
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

            YearEndIndex[i] = yearEnd;
            if (ts - beginOfYear(ts) < (int) p->getScheduleGranularity())
                yearEnd = i > 0 ? i - 1 : 0;
        }
    }

    for (int i = 0; i < 7; i++)
    {
        workingHours[i] = new QPtrList<Interval>();
        workingHours[i]->setAutoDelete(true);
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
                if (SB_IS_ALLOCATED(scoreboards[sc][i]))
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
                if (SB_IS_ALLOCATED(specifiedBookings[sc][i]))
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
            workingHours[i]->setAutoDelete(true);
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
            workingHours[i]->setAutoDelete(true);
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
        scoreboard[i] = SB_OFF;

    // Then change all worktime slots to 0 (available) again.
    for (time_t t = project->getStart(); t < project->getEnd() + 1;
         t += project->getScheduleGranularity())
    {
        if (isOnShift(Interval(t,
                               t + project->getScheduleGranularity() - 1)))
            scoreboard[sbIndex(t)] = SB_FREE;
    }
    // Then mark all resource specific vacation slots as such (2).
    for (QPtrListIterator<Interval> ivi(vacations); *ivi != 0; ++ivi)
        for (time_t date = (*ivi)->getStart() > project->getStart() ?
             (*ivi)->getStart() : project->getStart();
             date < (*ivi)->getEnd() && date < project->getEnd() + 1;
             date += project->getScheduleGranularity())
            scoreboard[sbIndex(date)] = SB_VACATION;

    // Mark all global vacation slots as such (2)
    for (VacationList::Iterator ivi(project->getVacationListIterator());
         *ivi != 0; ++ivi)
    {
        if ((*ivi)->getStart() > project->getEnd() ||
            (*ivi)->getEnd() < project->getStart())
            continue;
        uint startIdx = sbIndex((*ivi)->getStart() >= project->getStart() ?
                                (*ivi)->getStart() : project->getStart());
        uint endIdx = sbIndex((*ivi)->getEnd() <= project->getEnd() ?
                              (*ivi)->getEnd() : project->getEnd());
        for (uint idx = startIdx; idx <= endIdx; ++idx)
            scoreboard[idx] = SB_VACATION;
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
 * \retval BOOKING_FREE { slot is available }
 * \retval BOOKING_OFF { vacation/off duty }
 * \retval BOOKING_OVERLIMIT { resource overloaded }
 * \retval BOOKING_BOOKED { already booked }
 */
int
Resource::getBooking(time_t date)
{
    /* The scoreboard of a resource is only generated on demand, so that large
     * resource lists that are only scarcely used for the project do not slow
     * TJ down too much. */
    if (!scoreboard)
        initScoreboard();
    // Check if the interval is booked or blocked already.
    uint sbIdx = sbIndex(date);
    if (!SB_IS_FREE(scoreboard[sbIdx]))
    {
        if (DEBUGRS(6))
        {
            QString reason;
            if (scoreboard[sbIdx] == SB_OFF)
                reason = "off-hour";
            else if (scoreboard[sbIdx] == SB_VACATION)
                reason = "vacation";
            else if (scoreboard[sbIdx] == SB_OVERLIMIT)
                reason = "over usage limits";
            else if (SB_IS_UNAVAILABLE(scoreboard[sbIdx]))
                reason = "UNDEFINED";
            else
                reason = "allocated to " +
                    scoreboard[sbIdx]->getTask()->getId();
            qDebug("  Resource %s is busy (%s)", id.latin1(),
                   reason.latin1());
        }
        if (scoreboard[sbIdx] == SB_OVERLIMIT)
            return BOOKING_OVERLIMIT;
        if (SB_IS_UNAVAILABLE(scoreboard[sbIdx]))
            return BOOKING_OFF;
        return BOOKING_BOOKED;
    }

    if (!limits)
        return BOOKING_FREE;

    if ((limits && limits->getDailyMax() > 0))
    {
        // Now check that the resource is not overloaded on this day.
        uint bookedSlots = 1;

        for (uint i = DayStartIndex[sbIdx]; i <= DayEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (!SB_IS_ALLOCATED(b))
                continue;
            bookedSlots++;
        }

        if (limits && limits->getDailyMax() > 0 &&
            bookedSlots > limits->getDailyMax())
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded today (%d)", id.latin1(),
                       bookedSlots);
            scoreboard[sbIdx] = SB_OVERLIMIT;
            return BOOKING_OVERLIMIT;
        }
    }
    if ((limits && (limits->getWeeklyMax() > 0)
                    || limits->getWeeklyRatioMax() > 0.0))
    {
        // Now check that the resource is not overloaded on this week.
        uint bookedSlots = 1;
        uint workableSlots = 0;

        for (uint i = WeekStartIndex[sbIdx]; i <= WeekEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (SB_IS_FREE(b))
                workableSlots++;
             if (!SB_IS_ALLOCATED(b))
                continue;
            workableSlots++;
            bookedSlots++;
        }

        if (limits->getWeeklyMax() > 0 && bookedSlots > limits->getWeeklyMax()
            || limits->getWeeklyRatioMax() > 0.0 &&
               limits->getWeeklyRatioMax() < (double)bookedSlots/workableSlots)
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded this week (%d)", id.latin1(),
                       bookedSlots);
            scoreboard[sbIdx] = SB_OVERLIMIT;
            return BOOKING_OVERLIMIT;
        }
    }
    if ((limits && (limits->getMonthlyMax() > 0)
                    || limits->getMonthlyRatioMax() > 0.0))
    {
        // Now check that the resource is not overloaded on this month.
        uint bookedSlots = 1;
        uint workableSlots = 0;

        for (uint i = MonthStartIndex[sbIdx]; i <= MonthEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (SB_IS_FREE(b))
                workableSlots++;
            if (!SB_IS_ALLOCATED(b))
                continue;
            workableSlots++;
            bookedSlots++;
        }

        if (limits->getMonthlyMax() > 0 && bookedSlots > limits->getMonthlyMax()
            || limits->getMonthlyRatioMax() > 0.0 &&
               limits->getMonthlyRatioMax() < (double)bookedSlots/workableSlots)
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded this month (%d)", id.latin1(),
                       bookedSlots);
            scoreboard[sbIdx] = SB_OVERLIMIT;
            return BOOKING_OVERLIMIT;
        }
    }
    if ((limits && limits->getYearlyMax() > 0))
    {
        // Now check that the resource is not overloaded on this year.
        uint bookedSlots = 1;

        for (uint i = YearStartIndex[sbIdx]; i <= YearEndIndex[sbIdx]; i++)
        {
            SbBooking* b = scoreboard[i];
            if (!SB_IS_ALLOCATED(b))
                continue;

            bookedSlots++;
        }

        if (limits && limits->getYearlyMax() > 0 &&
            bookedSlots > limits->getYearlyMax())
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded this year (%d)", id.latin1(),
                       bookedSlots);
            scoreboard[sbIdx] = SB_OVERLIMIT;
            return BOOKING_OVERLIMIT;
        }
    }
    if ((limits && limits->getProjectMax() > 0))
    {
        // Now check that the resource is not overloaded on the whole project.
        uint bookedSlots = 1;

        for (uint i = 0; i < sbSize; i++)
        {
            SbBooking* b = scoreboard[i];
            if (!SB_IS_ALLOCATED(b))
                continue;

            bookedSlots++;
        }

        if (limits && limits->getProjectMax() > 0 &&
            bookedSlots > limits->getProjectMax())
        {
            if (DEBUGRS(6))
                qDebug("  Resource %s overloaded this project (%d)",
                       id.latin1(), bookedSlots);
            scoreboard[sbIdx] = SB_OVERLIMIT;
            return BOOKING_OVERLIMIT;
        }
    }

    return BOOKING_FREE;
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
        return false;
    }

    SbBooking* b;
    // Try to merge the booking with the booking in the previous slot.
    b = scoreboard[idx - 1];
    if (idx > 0 && SB_IS_ALLOCATED(b) && b->getTask() == nb->getTask())
    {
        scoreboard[idx] = b;
        delete nb;
        return true;
    }
    // Try to merge the booking with the booking in the following slot.
    b = scoreboard[idx + 1];
    if (idx < sbSize - 1 && SB_IS_ALLOCATED(b) && b->getTask() == nb->getTask())
    {
        scoreboard[idx] = b;
        delete nb;
        return true;
    }
    scoreboard[idx] = nb;
    return true;
}

bool
Resource::bookInterval(Booking* nb, int sc, int sloppy, int overtime)
{
    uint sIdx = sbIndex(nb->getStart());
    uint eIdx = sbIndex(nb->getEnd());

    bool conflict = false;

    for (uint i = sIdx; i <= eIdx; i++)
        if (scoreboard[i] > (SbBooking*) overtime)
        {
            uint j;
            for (j = i + 1 ; j <= eIdx &&
                 scoreboard[i] == scoreboard[j]; j++)
                ;
            if (scoreboard[i] == SB_OFF)
            {
                if (sloppy > 0)
                {
                    i = j;
                    continue;
                }
                TJMH.errorMessage
                    (i18n("Error in %1 scenario: "
                          "%2 has no duty hours at %3 "
                          "to be assigned to %4.")
                     .arg(project->getScenarioId(sc))
                     .arg(id).arg(time2ISO(index2start(i)))
                     .arg(nb->getTask()->getId().latin1()));
            }
            else if (scoreboard[i] == SB_VACATION)
            {
                if (sloppy > 1)
                {
                    i = j;
                    continue;
                }
                TJMH.errorMessage
                    (i18n("Error in %1 scenario: "
                          "%2 is on vacation at %3. "
                          "It cannot be assigned to %4.")
                     .arg(project->getScenarioId(sc))
                     .arg(id).arg(time2ISO(index2start(i)))
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
                          "Allocation conflict for %2 at %3. "
                          "Conflicting tasks are %4 and %5.")
                     .arg(project->getScenarioId(sc))
                     .arg(id).arg(time2ISO(index2start(i)))
                     .arg(scoreboard[i]->getTask()->getId().latin1())
                     .arg(nb->getTask()->getId().latin1()));
            }

            conflict = true;
            i = j;
        }

    if (conflict)
        return false;

    for (uint i = sIdx; i <= eIdx; i++)
        if (scoreboard[i] <= (SbBooking*) overtime)
            bookSlot(i, new SbBooking(*nb), overtime);

    return true;
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
        if (!SB_IS_ALLOCATED(b))
            continue;
        if (!task || task == b->getTask() || b->getTask()->isDescendantOf(task))
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

    uint bookedSlots = 0;

    for (uint i = DayStartIndex[sbIdx]; i <= DayEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (!SB_IS_ALLOCATED(b))
            continue;

        if (!t || b->getTask() == t || b->getTask()->isDescendantOf(t))
            bookedSlots++;
    }

    return bookedSlots;
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

    uint bookedSlots = 0;

    for (uint i = WeekStartIndex[sbIdx]; i <= WeekEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (!SB_IS_ALLOCATED(b))
            continue;

        if (!t || b->getTask() == t || b->getTask()->isDescendantOf(t))
            bookedSlots++;
    }

    return bookedSlots;
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

    uint bookedSlots = 0;

    for (uint i = MonthStartIndex[sbIdx]; i <= MonthEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (!SB_IS_ALLOCATED(b))
            continue;

        if (!t || b->getTask() == t || b->getTask()->isDescendantOf(t))
            bookedSlots++;
    }

    return bookedSlots;
}

uint
Resource::getCurrentYearSlots(time_t date, const Task* t)
{
    /* Return the number of slots this resource is allocated to in the current
     * scenario. If a task is given, only the slots allocated to this task
     * will be counted. */

    if (hasSubs())
    {
        uint timeSlots = 0;
        for (ResourceListIterator rli(getSubListIterator()); *rli; ++rli)
            timeSlots += (*rli)->getCurrentYearSlots(date, t);
        return timeSlots;
    }

    if (!scoreboard)
        return 0;

    uint sbIdx = sbIndex(date);

    uint bookedSlots = 0;

    for (uint i = YearStartIndex[sbIdx]; i <= YearEndIndex[sbIdx]; i++)
    {
        SbBooking* b = scoreboard[i];
        if (!SB_IS_ALLOCATED(b))
            continue;

        if (!t || b->getTask() == t || b->getTask()->isDescendantOf(t))
            bookedSlots++;
    }

    return bookedSlots;
}

uint
Resource::getProjectSlots(const Task* t)
{
    /* Return the number of slots this resource is allocated to in the current
     * scenario. If a task is given, only the slots allocated to this task
     * will be counted. */

    if (hasSubs())
    {
        uint timeSlots = 0;
        for (ResourceListIterator rli(getSubListIterator()); *rli; ++rli)
            timeSlots += (*rli)->getProjectSlots(t);
        return timeSlots;
    }

    if (!scoreboard)
        return 0;

    uint bookedSlots = 0;

    for (uint i = 0; i < sbSize; i++)
    {
        SbBooking* b = scoreboard[i];
        if (!SB_IS_ALLOCATED(b))
            continue;

        if (!t || b->getTask() == t || b->getTask()->isDescendantOf(t))
            bookedSlots++;
    }

    return bookedSlots;
}

double
Resource::getEffectiveLoad(int sc, const Interval& period, AccountType acctType,
                           const Task* task) const
{
    double load = 0.0;
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return 0.0;

    if (hasSubs())
    {
        for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
            load += (*rli)->getEffectiveLoad(sc, iv, acctType, task);
    }
    else
    {
        uint startIdx = sbIndex(iv.getStart());
        uint endIdx = sbIndex(iv.getEnd());
        load = project->convertToDailyLoad
            (getAllocatedSlots(sc, startIdx, endIdx, acctType, task) *
             project->getScheduleGranularity()) * efficiency;
    }

    return project->quantizeLoad(load);
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

    if (scenarios[sc].firstSlot > 0 && scenarios[sc].lastSlot > 0)
    {
        if (task)
        {
            /* If the load is to be calculated for a certain task, we check
             * whether this task is in the resource allocation list. Only then
             * we calculate the real number of allocated slots. */
            bool isAllocated = false;
            for (TaskListIterator tli(scenarios[sc].allocatedTasks); *tli;
                 ++tli)
                if (task == *tli || (*tli)->isDescendantOf(task))
                {
                    isAllocated = true;
                    break;
                }
            if (!isAllocated)
                return bookings;
        }

        if (startIdx < (uint) scenarios[sc].firstSlot)
            startIdx = scenarios[sc].firstSlot;
        if (endIdx > (uint) scenarios[sc].lastSlot)
            endIdx = scenarios[sc].lastSlot;
    }
    for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (!SB_IS_ALLOCATED(b))
            continue;
        if ((task == 0 ||
             (task != 0 && (task == b->getTask() ||
                            b->getTask()->isDescendantOf(task)))) &&
            (acctType == AllAccounts ||
             (b->getTask()->getAccount() &&
              b->getTask()->getAccount()->getAcctType() == acctType)))
            bookings++;
    }

    return bookings;
}

double
Resource::getEffectiveFreeLoad(int sc, const Interval& period)
{
    double load = 0.0;
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return 0.0;

    if (hasSubs())
    {
        for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
            load += (*rli)->getEffectiveFreeLoad(sc, iv);
    }
    else
    {
        uint startIdx = sbIndex(iv.getStart());
        uint endIdx = sbIndex(iv.getEnd());
        load = project->convertToDailyLoad
            (getAvailableSlots(sc, startIdx, endIdx) *
             project->getScheduleGranularity()) * efficiency;
    }

    return load;
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
        return false;

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
        return false;

    return isAllocatedSub(sc, startIdx, endIdx, prjId);
}

bool
Resource::isAllocatedSub(int sc, uint startIdx, uint endIdx, const QString&
                         prjId) const
{
    /* If resource is a group, check members first. */
    for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
        if ((*rli)->isAllocatedSub(sc, startIdx, endIdx, prjId))
            return true;

    if (!scoreboards[sc])
        return false;
    for (uint i = startIdx; i <= endIdx; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (!SB_IS_ALLOCATED(b))
            continue;
        if (prjId.isNull() || b->getTask()->getProjectId() == prjId)
            return true;
    }
    return false;
}

bool
Resource::isAllocated(int sc, const Interval& period, const Task* task) const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return false;

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
        return false;

    return isAllocatedSub(sc, startIdx, endIdx, task);
}

bool
Resource::isAllocatedSub(int sc, uint startIdx, uint endIdx, const Task* task)
    const
{
    /* If resource is a group, check members first. */
    for (ResourceListIterator rli(*sub); *rli != 0; ++rli)
        if ((*rli)->isAllocatedSub(sc, startIdx, endIdx, task))
            return true;

    if (!scoreboards[sc])
        return false;
    for (uint i = startIdx; i <= endIdx; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (!SB_IS_ALLOCATED(b))
            continue;
        if (!task || b->getTask() == task || b->getTask()->isDescendantOf(task))
            return true;
    }
    return false;
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
        if (!SB_IS_ALLOCATED(b))
            continue;
        if ((!task || task == b->getTask() ||
             b->getTask()->isDescendantOf(task)) &&
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
Resource::hasVacationDay(time_t day) const
{
    Interval fullDay(midnight(day),
                     sameTimeNextDay(midnight(day)) - 1);
    for (QPtrListIterator<Interval> vli(vacations); *vli != 0; ++vli)
        if ((*vli)->overlaps(fullDay))
            return true;

    if (shifts.isVacationDay(day))
        return true;

    if (workingHours[dayOfWeek(day, false)]->isEmpty())
        return true;

    return false;
}

bool
Resource::isOnShift(const Interval& slot) const
{
    for (ShiftSelectionList::Iterator ssli(shifts); *ssli != 0; ++ssli)
        if ((*ssli)->getPeriod().contains(slot))
            return (*ssli)->getShift()->isOnShift(slot);

    int dow = dayOfWeek(slot.getStart(), false);
    for (QPtrListIterator<Interval> ivi(*workingHours[dow]); *ivi != 0; ++ivi)
        if ((*ivi)->contains(Interval(secondsOfDay(slot.getStart()),
                                      secondsOfDay(slot.getEnd()))))
            return true;

    return false;
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
                if (!SB_IS_FREE(b))
                    bl.append(new Booking(Interval(index2start(startIdx),
                                                   index2end(i - 1)),
                                          scoreboards[sc][startIdx]));
                if (SB_IS_ALLOCATED(scoreboards[sc][i]))
                {
                    b = scoreboards[sc][i];
                    startIdx = i;
                }
                else
                    b = SB_FREE;
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
        if (SB_IS_ALLOCATED(scoreboards[sc][i]) &&
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
        if (SB_IS_ALLOCATED(scoreboards[sc][i]) &&
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
            if (SB_IS_ALLOCATED(dst[sc][i]))
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
            if (SB_IS_ALLOCATED(src[sc][i]))
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
        dst[sc] = SB_FREE;
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

    updateSlotMarks(sc);
}

void
Resource::finishScenario(int sc)
{
    scoreboards[sc] = scoreboard;
    updateSlotMarks(sc);
}

bool
Resource::bookingsOk(int sc)
{
    if (scoreboards[sc] == NULL)
        return true;

    if (hasSubs())
    {
       TJMH.errorMessage
          (i18n("Group resource '%1' may not have bookings") .arg(id));
       return false;
    }

    for (uint i = 0; i < sbSize; ++i)
        if (SB_IS_ALLOCATED(scoreboards[sc][i]))
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
                return false;
            }
        }

    return true;
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

void
Resource::updateSlotMarks(int sc)
{
    scenarios[sc].allocatedTasks.clear();
    scenarios[sc].firstSlot = -1;
    scenarios[sc].lastSlot = -1;

    if (scoreboard)
    {
        for (uint i = 0; i < sbSize; i++)
            if (SB_IS_ALLOCATED(scoreboard[i]))
            {
                if (scenarios[sc].firstSlot == -1)
                    scenarios[sc].firstSlot = i;
                scenarios[sc].lastSlot = i;
                scenarios[sc].addTask(scoreboard[i]->getTask());
            }
    }
}

QDomElement Resource::xmlIDElement( QDomDocument& doc ) const
{
   QDomElement elem = ReportXML::createXMLElem( doc, "Resource", getName());
   elem.setAttribute( "Id", getId() );

   return( elem );
}



