/*
 * ResourceList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>

#include "ResourceList.h"
#include "debug.h"
#include "TjMessageHandler.h"
#include "ShiftSelection.h"
#include "Task.h"
#include "Resource.h"
#include "Project.h"
#include "kotrus.h"

ResourceList::ResourceList()
{
	sorting[0] = CoreAttributesList::TreeMode;
	sorting[1] = CoreAttributesList::IdUp;
}

int
ResourceList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Resource* r1 = static_cast<Resource*>(i1);
	Resource* r2 = static_cast<Resource*>(i2);

	int res;
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
		if ((res = compareItemsLevel(r1, r2, i)) != 0)
			return res;
	return res;
}

bool
ResourceList::isSupportedSortingCriteria(int sc)
{
	switch (sc)
	{
	case TreeMode:
	case MinEffortUp:
	case MinEffortDown:
	case MaxEffortUp:
	case MaxEffortDown:
	case RateUp:
	case RateDown:
	case KotrusIdUp:
	case KotrusIdDown:
		return TRUE;
	default:
		return CoreAttributesList::isSupportedSortingCriteria(sc);
	}		
}

int
ResourceList::compareItemsLevel(Resource* r1, Resource* r2, int level)
{
	if (level < 0 || level >= maxSortingLevel)
		return -1;

	switch (sorting[level])
	{
	case TreeMode:
		if (level == 0)
			return compareTreeItemsT(this, r1, r2);
		else
			return r1->getSequenceNo() == r2->getSequenceNo() ? 0 :
				r1->getSequenceNo() < r2->getSequenceNo() ? -1 : 1;
	case MinEffortUp:
		return r1->minEffort == r2->minEffort ? 0 :
			r1->minEffort < r2->minEffort ? 1 : -1;
	case MinEffortDown:
		return r1->minEffort == r2->minEffort ? 0 :
			r1->minEffort < r2->minEffort ? -1 : 1;
	case MaxEffortUp:
		return r1->maxEffort == r2->maxEffort ? 0 :
			r1->maxEffort < r2->maxEffort ? 1 : -1;
	case MaxEffortDown:
		return r1->maxEffort == r2->maxEffort ? 0 :
			r1->maxEffort < r2->maxEffort ? -1 : 1;
	case RateUp:
		return r1->rate == r2->rate ? 0 : r1->rate < r2->rate ? 1 : -1;
	case RateDown:
		return r1->rate == r2->rate ? 0 : r1->rate < r2->rate ? -1 : 1;
	case KotrusIdUp:
		return r2->kotrusId.compare(r1->kotrusId);
	case KotrusIdDown:
		return r1->kotrusId.compare(r2->kotrusId);
	default:
		return CoreAttributesList::compareItemsLevel(r1, r2, level);
	}
}

Resource*
ResourceList::getResource(const QString& id) const
{
	for (ResourceListIterator rli(*this); *rli != 0; ++rli)
		if ((*rli)->getId() == id)
			return *rli;

	return 0;
}

ResourceTreeIterator::ResourceTreeIterator(Resource* r) :
	CoreAttributesTreeIterator(r) { }

