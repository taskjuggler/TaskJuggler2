/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _VacationList_h_
#define _VacationList_h_

#include "time.h"

#include <qlist.h>
#include <qstring.h>

#include "Interval.h"

/**
 * @short An interval with a name.
 * @author Chris Schlaeger <cs@suse.de>
 */
class VacationInterval : public Interval
{
public:
	VacationInterval() { }

	VacationInterval(const QString& n, time_t s, time_t e)
		: Interval(s, e), name(n) { }
	virtual ~VacationInterval() { }

	const QString& getName() const { return name; }

private:
	QString name;
} ;

/**
 * @short A list of vacations.
 * @author Chris Schlaeger <cs@suse.de>
 */
class VacationList : public QList<VacationInterval>
{
public:
	VacationList() { setAutoDelete(TRUE); }
	~VacationList() {}

	void add(const QString& name, time_t start, time_t end)
	{
		inSort(new VacationInterval(name, start, end));
	}
	bool isVacation(time_t date);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

#endif
