/*
 * ShiftSelectionList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ShiftSelectionList_h_
#define _ShiftSelectionList_h_

#include <time.h>

#include <qptrlist.h>

class ShiftSelection;
class Interval;

/**
 * @short Holds a list of shift selections.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ShiftSelectionList : public QPtrList<ShiftSelection>
{
public:
	ShiftSelectionList() { }
	virtual ~ShiftSelectionList() { }

	bool insert(ShiftSelection* s);

	bool isOnShift(const Interval& iv) const;

	bool isVacationDay(time_t day) const;

private:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
};

/**
 * @short Iterator for ShiftSelectionList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ShiftSelectionListIterator : public QPtrListIterator<ShiftSelection>
{
public:
	ShiftSelectionListIterator(const ShiftSelectionList& s) :
		QPtrListIterator<ShiftSelection>(s) { }
	~ShiftSelectionListIterator() { }
} ;

#endif

