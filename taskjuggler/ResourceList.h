/*
 * ResourceList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ResourceList_h_
#define _ResourceList_h_

#include <time.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qdom.h>

#include "Interval.h"
#include "VacationList.h"
#include "CoreAttributes.h"
#include "ShiftList.h"

class Project;
class Task;
class Resource;

class SbBooking
{
public:
	SbBooking(Task* t, QString a = "", QString i = "")
		: task(t), account(a), projectId(i) { }
	~SbBooking() { }
	
	Task* getTask() const { return task; }

	void setAccount(const QString a) { account = a; }
	const QString& getAccount() const { return account; }

	void setProjectId(const QString i) { projectId = i; }
	const QString& getProjectId() const { return projectId; }

private:
	// A pointer to the task that caused the booking
	Task* task;
	// String identifying the KoTrus account the effort is credited to.
	QString account;
	// The Project ID
	QString projectId;
};

class Booking : public SbBooking
{
public:
	Booking(const Interval& iv, Task* t, QString a = "",
			QString i = "")
		: SbBooking(t, a, i), interval(iv) { }
	Booking(const Interval& iv, SbBooking* sb) : SbBooking(*sb),
			interval(iv) { }
	~Booking() { }

	time_t getStart() const { return interval.getStart(); }
	time_t getEnd() const { return interval.getEnd(); }
	time_t getDuration() const { return interval.getDuration(); }
	Interval& getInterval() { return interval; }

	void setLockTS( const QString& ts ) { lockTS = ts; }
	const QString& getLockTS() const { return lockTS; }

	void setLockerId( const QString& id ) { lockerId = id; }
	const QString& getLockerId() const { return lockerId; }

private:
	// The booked time period.
	Interval interval;
	// The database lock timestamp
	QString lockTS;

	// the lockers ID
	QString lockerId;
} ;

class BookingList : public QPtrList<Booking>
{
public:
	BookingList() { }
	~BookingList() { }

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
};

typedef QPtrListIterator<Booking> BookingListIterator;

class Resource;

class ResourceList : public CoreAttributesList
{
public:
	ResourceList();
	~ResourceList() { }

	Resource* first() { return (Resource*) CoreAttributesList::first(); }
	Resource* next() { return (Resource*) CoreAttributesList::next(); }

	Resource* getResource(const QString& id);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
	virtual int compareItemsLevel(Resource* r1, Resource* r2, int level);
} ;

class Resource : public CoreAttributes
{
	friend int ResourceList::compareItemsLevel(Resource* r1, Resource* r2,
											   int level);
public:
	Resource(Project* p, const QString& i, const QString& n, Resource* p);
	virtual ~Resource();

	virtual const char* getType() { return "Resource"; }

	Resource* getParent() const { return (Resource*) parent; }

	bool isGroup() const { return !sub.isEmpty(); }
	void getSubResourceList(ResourceList& rl);

	Resource* subResourcesFirst();
	Resource* subResourcesNext();

	void setMinEffort(double e) { minEffort = e; }
	double getMinEffort() const { return minEffort; }

	void setMaxEffort(double e) { maxEffort = e; }
	double getMaxEffort() const { return maxEffort; }

	void setEfficiency(double e) { efficiency = e; }
	double getEfficiency() const { return efficiency; }

	void setRate(double r) { rate = r; }
	double getRate() const { return rate; }

	void addVacation(Interval* i) { vacations.append(i); }
	bool hasVacationDay(time_t day);

	bool isOnShift(const Interval& slot);

	void setWorkingHours(int day, QPtrList<Interval>* l)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		delete workingHours[day];
		workingHours[day] = l;
	}

	bool addShift(const Interval& i, Shift* s)
	{
		return shifts.insert(new ShiftSelection(i, s));
	}

	bool isAvailable(time_t day, time_t duration, int loadFactor, Task* t);

	bool book(Booking* b);

	bool bookSlot(uint idx, SbBooking* nb);
	bool bookInterval(Booking* b);
	bool addPlanBooking(Booking* b);
	bool addActualBooking(Booking* b);

	double getCurrentLoad(const Interval& i, Task* task = 0);

	long getCurrentLoadSub(uint startIdx, uint endIdx, Task* task);

	double getPlanLoad(const Interval& i, Task* task = 0);

	long getPlanLoadSub(uint startIdx, uint endIdx, Task* task);

	double getActualLoad(const Interval& i, Task* task = 0);

	long getActualLoadSub(uint startIdx, uint endIdx, Task* task);

	double getPlanCredits(const Interval& i, Task* task = 0);

	double getActualCredits(const Interval& i, Task* task = 0);

	QString getPlanProjectIDs(const Interval& i, Task* task = 0);
	QString getActualProjectIDs(const Interval& i, Task* task = 0);

	void setKotrusId(const QString k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	bool dbLoadBookings(const QString& kotrusID,
					   	const QStringList& skipProjectIDs);

	QDomElement xmlIDElement( QDomDocument& doc ) const;

	BookingList getPlanJobs();
	BookingList getActualJobs();

	void preparePlan();
	void finishPlan();

	void prepareActual();
	void finishActual();

	static void setDebugLevel(int l) { debugLevel = l; }

private:
	Resource* subFirst() { return (Resource*) sub.first(); }
	Resource* subNext() { return (Resource*) sub.next(); }

	void getPlanPIDs(const Interval& period, Task* task, QStringList& pids);
	void getActualPIDs(const Interval& period, Task* task, QStringList& pids);

	void initScoreboard();
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

	static int debugLevel;

	/// KoTrus ID, ID by which the resource is known to KoTrus.
	QString kotrusId;

	/// The list of standard working or opening hours for the resource.
	QList<Interval>* workingHours[7];

	/**
	 * In addition to the standard working hours a set of shifts can be
	 * defined. This is useful when the working hours change over time.
	 * A shift is only active in a defined interval. If no interval is
	 * defined for a period of time the standard working hours of the
	 * resource are used.
	 */
	ShiftSelectionList shifts;

	/// List of all intervals the resource is not available.
	QList<Interval> vacations;

	/**
	 * For each time slot (of length scheduling granularity) we store a
	 * pointer to a booking, a '1' if slot is off-hours, a '2' if slot is
	 * during a vacation or 0 if resource is available. */
	SbBooking** scoreboard;
	/// The number of time slots in the project.
	uint sbSize;

	/// Scoring of planned usages of the resource.
	SbBooking** planScoreboard;
	/// Scoring of actual usages of the resource.
	SbBooking** actualScoreboard;
} ;

#endif
