/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _VacationList_h_
#define _VacationList_h_

#include "time.h"

#include <qlist.h>
#include <qstring.h>

class Interval
{
public:
	Interval() { start = 0; end = 0; }
	Interval(const QString& n, time_t s, time_t e)
		: name(n), start(s), end(e) { }
	virtual ~Interval() { }

	const QString& getName() const { return name; }
	time_t getStart() const { return start; }
	time_t getEnd() const { return end; }

	QString name;
	time_t start;
	time_t end;
} ;

class VacationList : protected QList<Interval>
{
public:
	VacationList() { setAutoDelete(TRUE); }
	~VacationList() {}

	void add(const QString& name, time_t start, time_t end)
	{
		inSort(new Interval(name, start, end));
	}
	bool isVacationDay(time_t day);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

#endif
