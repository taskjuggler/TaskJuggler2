/*
 * ResourceList.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
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

class Project;
class Task;

class Booking
{
public:
	Booking(const Interval& iv, Task* t, QString a = "",
			QString i = "")
		: interval(iv), task(t), account(a), projectId(i) { }
	~Booking() { }

	time_t getStart() const { return interval.getStart(); }
	time_t getEnd() const { return interval.getEnd(); }
	time_t getDuration() const { return interval.getDuration(); }
	Interval& getInterval() { return interval; }

	Task* getTask() const { return task; }

	void setAccount(const QString a) { account = a; }
	const QString& getAccount() const { return account; }

	void setProjectId(const QString i) { projectId = i; }
	const QString& getProjectId() const { return projectId; }

	void setLockTS( const QString& ts ) { lockTS = ts; }
	const QString& getLockTS() const { return lockTS; }

	void setLockerId( const QString& id ) { lockerId = id; }
	const QString& getLockerId() const { return lockerId; }

private:
	// The booked time period.
	Interval interval;
	// A pointer to the task that caused the booking
	Task* task;
	// String identifying the KoTrus account the effort is credited to.
	QString account;
	// The Project ID
	QString projectId;
   
	// The database lock timestamp
	QString lockTS;

	// the lockers ID
	QString lockerId;

	// A list of flags that can be used to select a sub-set of tasks.
	FlagList flagList;
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
} ;

class Resource : public CoreAttributes
{
public:
	Resource(Project* p, const QString& i, const QString& n, Resource* p);
	virtual ~Resource() { }

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

	void setWorkingHours(int day, QPtrList<Interval>* l)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		delete workingHours[day];
		workingHours[day] = l;
	}
	bool isAvailable(time_t day, time_t duration, Interval& i);

	void book(Booking* b);

	double getPlanLoad(const Interval& i, Task* task = 0);

	double getActualLoad(const Interval& i, Task* task = 0);

	double getPlanCosts(const Interval& i, Task* task = 0);

	double getActualCosts(const Interval& i, Task* task = 0);

	QString getPlanProjectIDs(const Interval& i, Task* task = 0);
	QString getActualProjectIDs(const Interval& i, Task* task = 0);

	void setKotrusId(const QString k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	bool dbLoadBookings(const QString& kotrusID, const QString& skipProjectID);

        QDomElement xmlIDElement( QDomDocument& doc ) const
        {
	   
	   QDomElement elem = doc.createElement( "ResourceID" );
	   QDomText t=doc.createTextNode( getId() );
	   elem.appendChild( t );
	   elem.setAttribute( "Name", getName() );

	   return( elem );
        }

	BookingList getPlanJobs() { return planJobs; }
	BookingList getActualJobs() { return actualJobs; }

	void preparePlan();
	void finishPlan();

	void prepareActual();
	void finishActual();

private:
	Resource* subFirst() { return (Resource*) sub.first(); }
	Resource* subNext() { return (Resource*) sub.next(); }

	void getPlanPIDs(const Interval& period, Task* task, QStringList& pids);
	void getActualPIDs(const Interval& period, Task* task, QStringList& pids);

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

	/// The list of working or opening hours for the resource.
	QList<Interval>* workingHours[7];

	/// List of all intervals the resource is not available.
	QList<Interval> vacations;

	/// A list of all planned usages of the resource.
	BookingList planJobs;

	/// A list of all actual usages of the resource.
	BookingList actualJobs;

	/// A list of all scheduled uses of the resource.
	BookingList jobs;
} ;

#endif
