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
#include "FlagList.h"

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

class ResourceList : public QPtrList<Resource>
{
public:
	ResourceList();
	~ResourceList() { }

	enum SortCriteria { Pointer, Index, ResourceTree };

	Resource* getResource(const QString& id);

	void setSorting(SortCriteria sc) { sorting = sc; }

	void createIndex();

	void printText();

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);

private:
	SortCriteria sorting;
} ;

class Resource : public FlagList
{
public:
	Resource(Project* p, const QString& i, const QString& n, Resource* p);
	virtual ~Resource() { }

	const QString& getId() const { return id; }

	void setIndex(uint idx) { index = idx; }
	uint getIndex() const { return index; }

	const QString& getName() const { return name; }
	void getFullName(QString& fullName)
	{
		fullName = "";
		for (Resource* r = this; r != 0; r = r->parent)
			fullName = r->name + "." + fullName;
		fullName.remove(fullName.length() - 1, 1);
	}

	Resource* getParent() const { return parent; }

	bool isGroup() const { return !subResources.isEmpty(); }
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

	void setKotrusId(const QString k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	void printText();

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
	/// The ID of the resource. Must be unique in the project.
	Project* project;

	/// The ID of the resource. Must be unique in the project.
	QString id;

	/// A unique integer ID.
	uint index;

	/// The resource name. E. g. real name or room number.
	QString name;

	/**
	 * A resource can have sub-resources. This can be used to model teams.
	 * The resources that form the team can be used individually as well.
	 * This cannot be done if teams are modelled by setting an increased
	 * efficiency. */
	Resource* parent;

	/// List of resources that form this resource.
	ResourceList subResources;

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
