/*
 * CoreAttributesList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _CoreAttributesList_h_
#define _CoreAttributesList_h_

#include <qptrlist.h>

class CoreAttributes;

/**
 * @short The class stores a list of CoreAttributes.
 * @see CoreAttributes 
 * @author Chris Schlaeger <cs@suse.de>
 */
class CoreAttributesList : public QPtrList<CoreAttributes>
{
public:
	CoreAttributesList()
   	{
		for (int i = 0; i < maxSortingLevel; i++)
			sorting[i] = SequenceUp;
   	}
	CoreAttributesList(const CoreAttributesList& l) :
		QPtrList<CoreAttributes>(l)
	{
		for (int i = 0; i < maxSortingLevel; i++)
			sorting[i] = l.sorting[i];
	}
	virtual ~CoreAttributesList();

	enum SortCriteria {
	   	SequenceUp = 0, SequenceDown,
		TreeMode, NameUp, NameDown, FullNameUp,
		FullNameDown, IdUp, IdDown, IndexUp, IndexDown, 
		StatusUp, StatusDown, CompletedUp, CompletedDown,
		PrioUp, PrioDown,
		ResponsibleUp, ResponsibleDown,
		MinEffortUp, MinEffortDown,
		MaxEffortUp, MaxEffortDown,
		RateUp, RateDown,
		KotrusIdUp, KotrusIdDown,
		StartUp, StartDown, EndUp, EndDown
	};

	static const int maxSortingLevel = 3;
	void setSorting(int s, int level);
	void createIndex(bool initial = FALSE);
	uint maxDepth() const;

	static bool isSupportedSortingCriteria(int sc);

	virtual int compareItemsLevel(CoreAttributes* c1, CoreAttributes* c2,
								  int level);
	
protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);

	int sorting[maxSortingLevel];
} ;

/**
 * @short Iterator for CoreAttributesList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class CoreAttributesListIterator : public QPtrListIterator<CoreAttributes>
{
public:
	CoreAttributesListIterator(const CoreAttributesList& l) :
		QPtrListIterator<CoreAttributes>(l) { }
	virtual ~CoreAttributesListIterator() { }
} ;

#endif
