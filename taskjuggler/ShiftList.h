/*
 * ShiftList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ShiftList_h_
#define _ShiftList_h_

#include "CoreAttributesList.h"

class QString;
class Shift;
class Project;

/**
 * @short Stores a list of Shifts.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ShiftList : public CoreAttributesList
{
public:
	ShiftList()
	{
		sorting[0] = TreeMode;
		sorting[1] = SequenceUp;
	}
	virtual ~ShiftList() { }

	Shift* getShift(const QString& id) const;

	virtual int compareItemsLevel(Shift* s1, Shift* s2, int level);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator class for ShiftList objects.
 * @see ShiftList
 * @author Chris Schlaeger <cs@suse.de>
 */
class ShiftListIterator : public virtual CoreAttributesListIterator 
{
public:
	ShiftListIterator(const CoreAttributesList& l) :
		CoreAttributesListIterator(l) { }
	virtual ~ShiftListIterator() { }
	Shift* operator*() { return (Shift*) get(); }
} ;

#endif

