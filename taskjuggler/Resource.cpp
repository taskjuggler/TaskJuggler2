/*
 * Resource.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <assert.h>

#include "Resource.h"
#include "Project.h"
#include "Interval.h"
#include "VacationInterval.h"
#include "ShiftSelection.h"
#include "Booking.h"
#include "BookingList.h"
#include "SbBooking.h"
#include "Account.h"
#include "Task.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "kotrus.h"
#include "debug.h"
#include "ReportXML.h"
#include "CustomAttributeDefinition.h"

/*
 * Calls to sbIndex are fairly expensive due to the floating point
 * division. We therefor use a buffer that stores the index of the
 * first slot of a day for each slot.
 */
static int* MidnightIndex = 0;

Resource::Resource(Project* p, const QString& i, const QString& n,
                   Resource* pr)
    : CoreAttributes(p, i, n, pr)
{
    vacations.setAutoDelete(TRUE);
    shifts.setAutoDelete(TRUE);

    p->addResource(this);

    if (pr)
    {
        // Inherit flags from parent resource.
        for (QStringList::Iterator it = pr->flags.begin();
             it != pr->flags.end(); ++it)
            addFlag(*it);

        // Inherit default working hours from parent resource.
        for (int i = 0; i < 7; i++)
        {
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
        maxEffort = pr->maxEffort;
        rate = pr->rate;
        efficiency = pr->efficiency;

        // Inherit inheritable custom attributes
        inheritCustomAttributes(p->getResourceAttributeDict());
    }
    else
    {
        // Inherit from default working hours project defaults.
        for (int i = 0; i < 7; i++)
        {
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(TRUE);
            for (QPtrListIterator<Interval> ivi(p->getWorkingHoursIterator(i));
                 *ivi != 0; ++ivi)
                workingHours[i]->append(new Interval(**ivi));
        }

        minEffort = p->getMinEffort();
        maxEffort = p->getMaxEffort();
        rate = project->getRate();
        efficiency = 1.0;
    }

    sbSize = (p->getEnd() + 1 - p->getStart()) /
        p->getScheduleGranularity() + 1;
    
    for (int sc = 0; sc < p->getMaxScenarios(); sc++)
        scoreboards[sc] = 0;

    if (!MidnightIndex)
    {
        /*
         * Since we need to take daylight saving time switches into account
         * we have to add more than 24 hours to get to the next day. The
         * buffer must be big enough so we don't create overflows.
         */
        uint midnightIndexSize = sbSize + 
            (ONEDAY * 2) / p->getScheduleGranularity();
        MidnightIndex = new int[midnightIndexSize];
        for (uint i = 0; i < midnightIndexSize; i++)
            MidnightIndex[i] = -1;
    }
}

Resource::~Resource()
{
    int i;
    for (i = 0; i < 7; i++)
        delete workingHours[i];

    for (int sc = 0; sc < project->getMaxScenarios(); sc++)
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
    
    delete [] MidnightIndex;
    MidnightIndex = 0;
    project->deleteResource(this);
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
    for (VacationListIterator ivi(project->getVacationListIterator());
         *ivi != 0; ++ivi)
    {
        for (time_t date = (*ivi)->getStart();
             date < (*ivi)->getEnd() &&
                 project->getStart() <= date && date < project->getEnd() + 1;
             date += project->getScheduleGranularity())
            scoreboard[sbIndex(date)] = (SbBooking*) 2;
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

int
Resource::isAvailable(time_t date, time_t duration, int loadFactor,
                      const Task* t)
{
    /* The scoreboard of a resource is only generated on demand, so we large
     * resource lists that are only scarcely used for the project do not slow
     * it down too much. */
    if (!scoreboard)
        initScoreboard();
    // Check if the interval is booked or blocked already.
    uint sbIdx = sbIndex(date);
    if (scoreboard[sbIdx])
    {
        if (DEBUGRS(6))
            qDebug("  Resource %s is busy (%ld)", id.latin1(), (long)
                   scoreboard[sbIdx]);
        return scoreboard[sbIdx] < ((SbBooking*) 4) ? 1 : 4;
    }

    if (maxEffort == 0.0 && loadFactor == 0)
        return 0;

    // Now check that the resource is not overloaded on this day.
    time_t bookedTime = duration;
    time_t bookedTimeTask = duration;

    uint sbStart;
    if (midnight(date) <= project->getStart())
        sbStart = 0;
    else
    {
        if (MidnightIndex[sbIdx] == -1)
            MidnightIndex[sbIdx] = sbIndex(midnight(date));
        sbStart = MidnightIndex[sbIdx];
    }

    uint sbEnd;
    if (sameTimeNextDay(midnight(date)) >= project->getEnd())
        sbEnd = sbSize - 1;
    else
    {
        uint sbIdxEnd = sbStart +
            (uint) ((ONEDAY / project->getScheduleGranularity()) * 1.5);    
        if (MidnightIndex[sbIdxEnd] == -1)
            MidnightIndex[sbIdxEnd] = sbIndex(sameTimeNextDay(midnight(date)));
        sbEnd = MidnightIndex[sbIdxEnd] - 1;
    }
    
    for (uint i = sbStart; i < sbEnd; i++)
    {
        SbBooking* b = scoreboard[i];
        if (b < (SbBooking*) 4)
            continue;

        bookedTime += duration;
        if (b->getTask() == t)
            bookedTimeTask += duration;
    }

    double resourceLoad = project->convertToDailyLoad(bookedTime) * efficiency;
    double taskLoad = project->convertToDailyLoad(bookedTimeTask) * efficiency;
    if (DEBUGRS(7))
    {
        qDebug("  Resource %s: RLoad: %f(%f), TLoad: %f(%f)",
               id.latin1(), resourceLoad, maxEffort, taskLoad, loadFactor /
               100.0);
    }
    if (maxEffort > 0.0 && resourceLoad > maxEffort)
    {
        if (DEBUGRS(6))
            qDebug("  Resource %s overloaded (%f)", id.latin1(), resourceLoad);
        return 2;
    }
    if (loadFactor > 0 && taskLoad > (loadFactor / 100.0))
    {
        if (DEBUGRS(6))
            qDebug("  %s overloaded for task %s (%f)", id.latin1(),
                   t->getId().latin1(), loadFactor / 100.0);
        return 3;
    }
    return 0;
}

bool
Resource::book(Booking* nb)
{
    uint idx = sbIndex(nb->getStart());

    return bookSlot(idx, nb);
}

bool
Resource::bookSlot(uint idx, SbBooking* nb)
{
    // Test if the time slot is still available.
    if (scoreboard[idx] != 0)
        return FALSE;
    
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
Resource::bookInterval(Booking* nb, int sc, int sloppy)
{
    uint sIdx = sbIndex(nb->getStart());
    uint eIdx = sbIndex(nb->getEnd());

    bool conflict = FALSE;
    
    for (uint i = sIdx; i <= eIdx; i++)
        if (scoreboard[i])
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
        bookSlot(i, new SbBooking(*nb));

    return TRUE;
}

bool
Resource::addBooking(int sc, Booking* nb, int sloppy)
{
    SbBooking** tmp = scoreboard;
    
    if (scoreboards[sc])
        scoreboard = scoreboards[sc];
    else
        initScoreboard();
    bool retVal = bookInterval(nb, sc, sloppy);
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

void 
Resource::addVacation(Interval* i)
{
    vacations.append(i); 
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

    for (ResourceListIterator rli(sub); *rli != 0; ++rli)
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

double
Resource::getLoad(int sc, const Interval& period, AccountType acctType,
                  const Task* task) const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
        return 0.0;

    return efficiency * project->convertToDailyLoad
        (getLoadSub(sc, sbIndex(iv.getStart()), sbIndex(iv.getEnd()),
                    acctType, task) * project->getScheduleGranularity());
}

long
Resource::getLoadSub(int sc, uint startIdx, uint endIdx, AccountType acctType,
                     const Task* task) const
{
    long bookings = 0;

    for (ResourceListIterator rli(sub); *rli != 0; ++rli)
        bookings += (*rli)->getLoadSub(sc, startIdx, endIdx, acctType, task);

    // If the scoreboard has not been initialized there is no load.
    if (!scoreboards[sc])
        return bookings;
    
    for (uint i = startIdx; i <= endIdx && i < sbSize; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (b < (SbBooking*) 4)
            continue;
        if ((task == 0 || 
            (task != 0 && task == b->getTask()) && 
             (acctType == AllAccounts || 
              b->getTask()->getAccount()->getAcctType() == acctType)))
            bookings++;
    }

    return bookings;
}

double
Resource::getCredits(int sc, const Interval& period, AccountType acctType,
                     const Task* task) const
{
    return getLoad(sc, period, acctType, task) * rate;
}

bool
Resource::isAllocated(int sc, const Interval& period, const QString& prjId)
    const
{
    Interval iv(period);
    if (!iv.overlap(Interval(project->getStart(), project->getEnd())))
    {
        qDebug("%s - %s", time2ISO(period.getStart()).latin1(),
               time2ISO(period.getEnd()).latin1());

        return FALSE;
    }

    /* If resource is a group, check members first. */
    for (ResourceListIterator rli(sub); *rli != 0; ++rli)
        if ((*rli)->isAllocated(sc, iv, prjId))
            return TRUE;

    if (!scoreboards[sc])
        return FALSE;
    for (uint i = sbIndex(iv.getStart());
         i <= sbIndex(iv.getEnd()) && i < sbSize; i++)
    {
        SbBooking* b = scoreboards[sc][i];
        if (b < (SbBooking*) 4)
            continue;
        if (prjId.isNull() || b->getTask()->getProjectId() == prjId)
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

    for (ResourceListIterator rli(sub); *rli != 0; ++rli)
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
        pidStr += QString(it != pids.begin() ? ", " : "") +
            project->getIdIndex(*it);

    return pidStr;
}

/* retrieve all bookings _not_ belonging to this project */
bool
Resource::dbLoadBookings(const QString& kotrusID,
                         const QStringList& skipProjectIDs)
{
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
    for (ShiftSelectionListIterator ssli(shifts); *ssli != 0; ++ssli)
        if ((*ssli)->getPeriod().contains(slot))
            return (*ssli)->getShift()->isOnShift(slot);

    int dow = dayOfWeek(slot.getStart(), FALSE);
    for (QPtrListIterator<Interval> ivi(*workingHours[dow]); *ivi != 0; ++ivi)
        if ((*ivi)->contains(Interval(secondsOfDay(slot.getStart()),
                                      secondsOfDay(slot.getEnd()))))
            return TRUE;

    return FALSE;
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
    for (int i = 0; i < sbSize; ++i)
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
Resource::prepareScenario(int sc)
{
    scoreboard = scoreboards[sc];
}

void
Resource::finishScenario(int sc)
{
    scoreboards[sc] = scoreboard;
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

QDomElement Resource::xmlIDElement( QDomDocument& doc ) const
{
   QDomElement elem = ReportXML::createXMLElem( doc, "Resource", getName());
   elem.setAttribute( "Id", getId() );
   
   return( elem );
}



