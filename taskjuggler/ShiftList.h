/*
 * ShiftList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ShiftList_h_
#define _ShiftList_h_

#include <qptrlist.h>

#include "CoreAttributes.h"
#include "Interval.h"

class Shift;
class Project;

class ShiftList : public CoreAttributesList
{
public:
	ShiftList()
	{
		sorting[0] = TreeMode;
		sorting[1] = SequenceUp;
		sorting[2] = SequenceUp;
	}
	~ShiftList() { }

	Shift* first() { return (Shift*) CoreAttributesList::first(); }
	Shift* next() { return (Shift*) CoreAttributesList::next(); }

	Shift* getShift(const QString& id);

	virtual int compareItemsLevel(Shift* s1, Shift* s2, int level);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

class Shift : public CoreAttributes
{
public:
	Shift(Project* prj, const QString& i, const QString& n, Shift* p);
	virtual ~Shift();

	virtual const char* getType() { return "Shift"; }

	Shift* getParent() { return (Shift*) parent; }

	void setWorkingHours(int day, QPtrList<Interval>* l)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		delete workingHours[day];
		workingHours[day] = l;
	}

	QPtrList<Interval>* getWorkingHours(int day)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		return workingHours[day];
	}

	bool isOnShift(const Interval& iv);

	bool isVacationDay(time_t day)
	{
		return workingHours[dayOfWeek(day, FALSE)]->isEmpty();
	}

private:
	Shift() { }		// Don't use this.
	
	QPtrList<Interval>* workingHours[7];	
};

class ShiftSelection;

class ShiftSelectionList : public QPtrList<ShiftSelection>
{
public:
	ShiftSelectionList() { }
	virtual ~ShiftSelectionList() { }

	bool insert(ShiftSelection* s);

	bool isOnShift(const Interval& iv);

	bool isVacationDay(time_t day);

private:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
};

class ShiftSelection
{
	friend int ShiftSelectionList::compareItems(QCollection::Item i1, 
												QCollection::Item i2);

public:
	ShiftSelection(const Interval& p, Shift* s) : period(p), shift(s) { }
	~ShiftSelection() { }

	const Interval& getPeriod() const { return period; }
	Shift* getShift() const { return shift; }

	bool isVacationDay(time_t day);

private:
	Interval period;
	Shift* shift;
};

#endif

