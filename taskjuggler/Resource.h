/*
 * Resource.h - TaskJuggler
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
#ifndef _Resource_h_
#define _Resource_h_

#include "ResourceScenario.h"

class Project;
class Shift;
class Task;
class Booking;
class SbBooking;
class BookingList;
class Interval;
class QDomDocument;
class QDomElement;
class UsageLimits;

/**
 * @short Stores all information about a resource.
 * @author Chris Schlaeger <cs@kde.org>
 */
class Resource : public CoreAttributes
{
    friend int ResourceList::compareItemsLevel(CoreAttributes* c1,
                                               CoreAttributes* c2, int level);
public:
    Resource(Project* p, const QString& i, const QString& n, Resource* pr,
             const QString& df = QString::null, uint dl = 0);
    virtual ~Resource();

    static void deleteStaticData();

    virtual CAType getType() const { return CA_Resource; }

    Resource* getParent() const { return static_cast<Resource*>(parent); }

    ResourceListIterator getSubListIterator() const
    {
        return ResourceListIterator(*sub);
    }

    void inheritValues();

    bool isGroup() const { return !sub->isEmpty(); }

    void setMinEffort(double e) { minEffort = e; }
    double getMinEffort() const { return minEffort; }

    void setLimits(UsageLimits* l);

    const UsageLimits* getLimits() const { return limits; }

    void setEfficiency(double e) { efficiency = e; }
    double getEfficiency() const { return efficiency; }

    bool isWorker() const;

    void setRate(double r) { rate = r; }
    double getRate() const { return rate; }

    void addVacation(Interval* i);
    QPtrListIterator<Interval> getVacationListIterator() const
    {
        return QPtrListIterator<Interval>(vacations);
    }

    bool hasVacationDay(time_t day) const;

    bool isOnShift(const Interval& slot) const;

    void setWorkingHours(int day, const QPtrList<Interval>& l);
    const QPtrList<Interval>* const* getWorkingHours() const
    {
        return workingHours;
    }

    bool addShift(const Interval& i, Shift* s);
    bool addShift(ShiftSelection* s);

    const ShiftSelectionList* getShiftList() const
    {
        return &shifts;
    }

    /***
     * Check if the slot with the specified duration is booked already.
     * @ret BOOKING_FREE slot is available, BOOKING_OFF vacation/off duty,
     * BOOKING_OVERLIMIT resource overloaded, BOOKING_BOOKED already booked.
     * Nota: this method used to be (mis)called "isAvailable".
     */
    int getBooking(time_t day);
#define BOOKING_FREE 0
#define BOOKING_OFF 1
#define BOOKING_OVERLIMIT 2
#define BOOKING_BOOKED 4

    bool book(Booking* b);

    bool bookInterval(Booking* b, int sc, int sloppy = 0, int overtime = 0);
    bool addBooking(int sc, Booking* b, int sloppy = 0, int overtime = 0);

    double getCurrentLoad(const Interval& i, const Task* task = 0) const;

    uint getCurrentDaySlots(time_t date, const Task* t);
    uint getCurrentWeekSlots(time_t date, const Task* t);
    uint getCurrentMonthSlots(time_t date, const Task* t);
    uint getCurrentYearSlots(time_t date, const Task* t);
    uint getProjectSlots(const Task *t);

    /***
     * Return the load of the resource (and its children) weighted by their
     * efficiency.
     */
    double getEffectiveLoad(int sc, const Interval& i,
                            AccountType acctType = AllAccounts,
                             const Task* task = 0) const;
    double getAllocatedTimeLoad(int sc, const Interval& period,
                                AccountType acctType, const Task* task = 0)
        const;
    long getAllocatedTime(int sc, const Interval& period, AccountType acctType,
                          const Task* task = 0) const;

    /***
     * Return the unallocated load of the resource and its children wheighted
     * by their efficiency if effective == true and efficiency != 0.0.
     */
    double getFreeLoad(int sc, const Interval& period, bool effective);
    inline double getEffectiveFreeLoad(int sc, const Interval& period)
    {
        return getFreeLoad(sc, period, true);
    }
    double getOverLimitLoad(int sc, const Interval& period, bool effective);

    double getCredits(int sc, const Interval& i, AccountType acctType,
                      const Task* task = 0) const;

    QString getProjectIDs(int sc, const Interval& i, const Task* task = 0)
        const;

    bool isAllocated(int sc, const Interval& i,
                     const QString& prjId = QString::null) const;

    bool isAllocated(int sc, const Interval& i, const Task* t) const;

    BookingList getJobs(int sc) const;

    time_t getStartOfFirstSlot(int sc, const Task* task);
    time_t getEndOfLastSlot(int sc, const Task* task);

    QDomElement xmlIDElement( QDomDocument& doc ) const;

    void copyBookings(int sc, SbBooking*** srd, SbBooking*** dst);
    void saveSpecifiedBookings();
    void prepareScenario(int sc);
    void finishScenario(int sc);

    bool bookingsOk(int sc);

    void resetAllocationProbability(int sc) { allocationProbability[sc] = 0; }
    void addAllocationProbability(int sc, double ap)
    {
        allocationProbability[sc] += ap;
    }
    double getAllocationProbability(int sc) const
    {
        return allocationProbability[sc];
    }

    TaskListIterator getTaskListIterator(int sc) const
    {
        return TaskListIterator(scenarios[sc].allocatedTasks);
    }

    void addJournalEntry(JournalEntry* entry);

    bool hasJournal() const { return !journal.isEmpty(); }

    Journal::Iterator getJournalIterator() const;

private:
    void getPIDs(int sc, const Interval& period, const Task* task,
                 QStringList& pids) const;

    void initScoreboard();

    bool bookSlot(uint idx, SbBooking* nb, int overtime = 0);

    long getCurrentLoadSub(uint startIdx, uint endIdx, const Task* task) const;

    long getAllocatedSlots(int sc, uint startIdx, uint endIdx,
                           AccountType acctType, const Task* task) const;

    long getAvailableSlots(int sc, uint startIdx, uint endIdx);
    long getOverLimitSlots(int sc, uint startIdx, uint endIdx);
    void freezeScenario(int sc);

    bool isAllocatedSub(int sc, uint startIdx, uint endIdx, const QString&
                        prjId) const;
    bool isAllocatedSub(int sc, uint startIdx, uint endIdx, const Task* task)
        const;
    void updateSlotMarks(int sc);

    uint sbIndex(time_t date) const;

    time_t index2start(uint idx) const;
    time_t index2end(uint idx) const;

    /// The minimum effort (in man days) the resource should be used per day.
    double minEffort;

    /// List of notes with a date attached.
    Journal journal;

    /// Usage limits of the resource.
    UsageLimits* limits;

    /**
     * The efficiency of the resource. A team of five should have an
     * efficiency of 5.0 */
    double efficiency;

    /// The daily costs of this resource.
    double rate;

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
     * pointer to a booking, a SB_OFF if slot is off-hours, a SB_VACATION
     * if slot is during a vacation or SB_FREE if resource is available.
     *
     * SB_IS_FREE is true if the slot can be allocated to a task or when
     *   it has not yet been detected to be over the resource usage limits
     *   (by Resource::getBooking())
     * SB_IS_ALLOCATED is true when the slot is already allocated to a
     *   a task, in this case, the slot is a pointer to a valid SbBooking
     *   object
     * SB_IS_UNAVAILABLE is true when the slot is unavailable for any
     *   other reason than an allocation: off hours, vacations, resource
     *   usage limits expired or any future reason. */
    SbBooking** scoreboard;
#define SB_FREE (NULL)
#define SB_OFF ((SbBooking*)1)
#define SB_VACATION ((SbBooking*)2)
#define SB_OVERLIMIT ((SbBooking*)4)
#define SB_IS_FREE(value) ((value)==SB_FREE)
#define SB_IS_ALLOCATED(value) ((value)>SB_OVERLIMIT)
#define SB_IS_UNAVAILABLE(value) (!SB_IS_FREE(value)&&!SB_IS_ALLOCATED(value))

    /// The number of time slots in the project.
    uint sbSize;

    SbBooking*** specifiedBookings;
    SbBooking*** scoreboards;

    ResourceScenario* scenarios;

    bool* frozenScenarios;

    /**
     * The allocation probability is calculated prior to scheduling a
     * scenario. It is the expected average effort the resource has to deliver
     * based on the assignments to tasks, not taking parallel assignments into
     * account.
     */
    double* allocationProbability;
} ;

#endif

