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

	virtual const char* getType() const { return "Resource"; }

	Resource* getParent() const { return (Resource*) parent; }

	bool isGroup() const { return !sub.isEmpty(); }
	void getSubResourceList(ResourceList& rl) const;

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

	void addVacation(Interval* i);
	
	bool hasVacationDay(time_t day) const;

	bool isOnShift(const Interval& slot) const;

	void setWorkingHours(int day, QPtrList<Interval>* l)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		delete workingHours[day];
		workingHours[day] = l;
	}

	bool addShift(const Interval& i, Shift* s);

	bool isAvailable(time_t day, time_t duration, int loadFactor, Task* t)
		const;

	bool book(Booking* b);

	bool bookSlot(uint idx, SbBooking* nb);
	bool bookInterval(Booking* b);
	bool addBooking(int sc, Booking* b);

	double getCurrentLoad(const Interval& i, Task* task = 0) const;

	long getCurrentLoadSub(uint startIdx, uint endIdx, Task* task) const;

	double getLoad(int sc, const Interval& i, Task* task = 0) const;

	long getLoadSub(int sc, uint startIdx, uint endIdx, Task* task) const;

	double getCredits(int sc, const Interval& i, Task* task = 0) const;

	QString getProjectIDs(int sc, const Interval& i, Task* task = 0) const;

	bool isAllocated(int sc, const Interval& i, const QString& prjId) const;

	BookingList getJobs(int sc) const;

	void setKotrusId(const QString k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	bool dbLoadBookings(const QString& kotrusID,
					   	const QStringList& skipProjectIDs);

	QDomElement xmlIDElement( QDomDocument& doc ) const;

	void prepareScenario(int sc);
	void finishScenario(int sc);

private:
	Resource* subFirst() { return (Resource*) sub.first(); }
	Resource* subNext() { return (Resource*) sub.next(); }

	void getPIDs(int sc, const Interval& period, Task* task, 
				 QStringList& pids) const;

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

	SbBooking** scoreboards[2];
} ;

#endif

