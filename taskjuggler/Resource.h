/*
 * Resource.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Resource_h_
#define _Resource_h_

#include "time.h"

#include "taskjuggler.h"
#include "CoreAttributes.h"
#include "ResourceList.h"
#include "ShiftSelectionList.h"

class Project;
class Shift;
class Task;
class Booking;
class SbBooking;
class BookingList;
class Interval;
class QDomDocument;
class QDomElement;

/**
 * @short Stores all information about a resource.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Resource : public CoreAttributes
{
    friend int ResourceList::compareItemsLevel(Resource* r1, Resource* r2,
                                               int level);
public:
    Resource(Project* p, const QString& i, const QString& n, Resource* p);
    virtual ~Resource();

    virtual CAType getType() const { return CA_Resource; }

    Resource* getParent() const { return (Resource*) parent; }

    ResourceListIterator getSubListIterator() const
    {
        return ResourceListIterator(sub);
    }

    bool isGroup() const { return !sub.isEmpty(); }

    void setMinEffort(double e) { minEffort = e; }
    double getMinEffort() const { return minEffort; }

    void setMaxEffort(double e) { maxEffort = e; }
    double getMaxEffort() const { return maxEffort; }

    void setEfficiency(double e) { efficiency = e; }
    double getEfficiency() const { return efficiency; }

    void setRate(double r) { rate = r; }
    double getRate() const { return rate; }

    void addVacation(Interval* i);
    
    bool hasVacationDay(time_t day) const;

    bool isOnShift(const Interval& slot) const;

    void setWorkingHours(int day, QPtrList<Interval>* l)
    {
        delete workingHours[day];
        workingHours[day] = l;
    }
    const QPtrList<const Interval>* const* getWorkingHours() const
    {
        return (const QPtrList<const Interval>* const*) workingHours;
    }

    bool addShift(const Interval& i, Shift* s);

    const ShiftSelectionList* getShiftList() const
    {
        return &shifts;
    }

    /***
     * Check if the slot with the specified duration is booked already.
     * @ret 0 slot is available, 1 vacation/off duty, 2 resource overloaded,
     * 3 task overloaded, 4 booked for other task, 
     */
    int isAvailable(time_t day, time_t duration, int loadFactor,
                     const Task* t);

    bool book(Booking* b);

    bool bookSlot(uint idx, SbBooking* nb);
    bool bookInterval(Booking* b, int sc, int sloppy = 0);
    bool addBooking(int sc, Booking* b, int sloppy = 0);

    double getCurrentLoad(const Interval& i, const Task* task = 0) const;

    double getLoad(int sc, const Interval& i, 
                   AccountType acctType = AllAccounts,
                   const Task* task = 0) const;

    double getCredits(int sc, const Interval& i, AccountType acctType,
                      const Task* task = 0) const;

    QString getProjectIDs(int sc, const Interval& i, const Task* task = 0) 
        const;

    bool isAllocated(int sc, const Interval& i,
                     const QString& prjId = QString::null) const;

    BookingList getJobs(int sc) const;

    time_t getStartOfFirstSlot(int sc, const Task* task);
    time_t getEndOfLastSlot(int sc, const Task* task);

    void setKotrusId(const QString k) { kotrusId = k; }
    const QString& getKotrusId() const { return kotrusId; }

    bool dbLoadBookings(const QString& kotrusID,
                        const QStringList& skipProjectIDs);

    QDomElement xmlIDElement( QDomDocument& doc ) const;

    void prepareScenario(int sc);
    void finishScenario(int sc);

    bool bookingsOk(int sc);

private:
    void getPIDs(int sc, const Interval& period, const Task* task, 
                 QStringList& pids) const;

    void initScoreboard();
    long getCurrentLoadSub(uint startIdx, uint endIdx, const Task* task) const;

    long getLoadSub(int sc, uint startIdx, uint endIdx, AccountType acctType,
                    const Task* task) const;

    uint sbIndex(time_t date) const;

    time_t index2start(uint idx) const;
    time_t index2end(uint idx) const;

    /// Pointer used by subResourceFirst() and subResourceNext().
    Resource* currentSR;

    /// The minimum effort (in man days) the resource should be used per day.
    double minEffort;

    /// The maximum effort (in man days) the resource should be used per day.
    double maxEffort;

    /**
     * The efficiency of the resource. A team of five should have an
     * efficiency of 5.0 */
    double efficiency;

    /// The daily costs of this resource.
    double rate;

    /// KoTrus ID, ID by which the resource is known to KoTrus.
    QString kotrusId;

    /// The list of standard working or opening hours for the resource.
    QPtrList<Interval>* workingHours[7];

    /**
     * In addition to the standard working hours a set of shifts can be
     * defined. This is useful when the working hours change over time.
     * A shift is only active in a defined interval. If no interval is
     * defined for a period of time the standard working hours of the
     * resource are used.
     */
    ShiftSelectionList shifts;

    /// List of all intervals the resource is not available.
    QPtrList<Interval> vacations;

    /**
     * For each time slot (of length scheduling granularity) we store a
     * pointer to a booking, a '1' if slot is off-hours, a '2' if slot is
     * during a vacation or 0 if resource is available. */
    SbBooking** scoreboard;
    /// The number of time slots in the project.
    uint sbSize;

    SbBooking*** scoreboards;
} ;

#endif

