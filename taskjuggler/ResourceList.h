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

#include "Interval.h"
#include "VacationList.h"

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
} ;

class BookingList : public QPtrList<Booking>
{
public:
	BookingList() { setAutoDelete(TRUE); }
	~BookingList() {}

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
};

typedef QPtrListIterator<Booking> BookingListIterator;

class Resource
{
public:
	Resource(Project* p, const QString& i, const QString& n, double mie = 0.0,
			 double mae = 1.0, double rate = 0.0);
	virtual ~Resource() { }

	const QString& getId() const { return id; }
	const QString& getName() const { return name; }

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

	void setWorkingHours(int day, const QPtrList<Interval>& l)
	{
		if (day < 0 || day > 7)
			qFatal("day out of range");
		workingHours[day] = l;
	}
	bool isAvailable(time_t day, time_t duration, Interval& i);

	void book(Booking* b);

	double getLoadOnDay(time_t date, Task* task = 0);

	bool isAssignedTo(Task* t);
	void setKotrusId(const QString k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	void printText();

	bool dbLoadBookings(const QString& kotrusID, const QString& skipProjectID);

private:
	// The ID of the resource. Must be unique in the project.
	Project* project;

	// The ID of the resource. Must be unique in the project.
	QString id;
	// The resource name. E. g. real name or room number.
	QString name;

	// The minimum effort (in man days) the resource should be used per day.
	double minEffort;
	// The maximum effort (in man days) the resource should be used per day.
	double maxEffort;
	/* The efficiency of the resource. A team of five should have 
	 * an efficiency of 5.0 */
	double efficiency;

	// The daily costs of this resource.
	double rate;

	// KoTrus ID, ID by which the resource is known to KoTrus.
	QString kotrusId;

	// The list of working or opening hours for the resource.
	QList<Interval> workingHours[7];

	// List of all intervals the resource is not available.
	QList<Interval> vacations;

	// A list of all uses of the resource.
	BookingList jobs;
} ;

class ResourceList : public QPtrList<Resource>
{
public:
	ResourceList() { setAutoDelete(TRUE); }
	~ResourceList() { }

	Resource* getResource(const QString& id);

	void printText();

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

#endif
