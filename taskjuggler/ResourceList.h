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
#include <qlist.h>
#include <qstring.h>

#include "VacationList.h"

class Project;
class Task;

class Booking
{
public:
	Booking(time_t d, Task* t, double e) : date(d), task(t), effort(e) { }
	~Booking() { }

	time_t getDate() const { return date; }
	Task* getTask() const { return task; }
	double getEffort() const { return effort; }

private:
	time_t date;
	Task* task;
	double effort;
} ;

class Resource
{
public:
	Resource(const QString& i, const QString& n, double mie = 0.0,
			 double mae = 1.0, double r = 0.0) :
		id(i), name(n), minEffort(mie), maxEffort(mae), rate(r) { }
	virtual ~Resource() { }

	const QString& getId() const { return id; }
	const QString& getName() const { return name; }

	void setMinEffort(double e) { minEffort = e; }
	double getMinEffort() const { return minEffort; }

	void setMaxEffort(double e) { maxEffort = e; }
	double getMaxEffort() const { return maxEffort; }

	void setRate(double r) { rate = r; }
	double getRate() const { return rate; }

	double isAvailable(time_t date);
	bool book(Booking* b);

	void printText();

private:
	QString id;
	QString name;
	double minEffort;
	double maxEffort;
	double rate;
	VacationList vacationList;
	QList<Booking> jobs;
} ;

class ResourceList : protected QList<Resource>
{
public:
	ResourceList() { setAutoDelete(TRUE); }
	~ResourceList() { }

	void add(Resource* r);
	Resource* getResource(const QString& id);

	void printText();

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

#endif
