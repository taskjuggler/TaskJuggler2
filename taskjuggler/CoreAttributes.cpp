/*
 * CoreAttributes.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CoreAttributes.h"

CoreAttributesList::~CoreAttributesList()
{
}

void
CoreAttributesList::setSorting(SortCriteria s, int level)
{
	if (level >=0 && level < maxSortingLevel)
		sorting[level] = s;
	else
		qFatal("CoreAttributesList::setSorting: level out of range: %d",
			   level);
}

void
CoreAttributesList::createIndex(bool initial)
{
	/* In "initial" mode the sequenceNo is set. This should only be done once
	 * for each list. In the other mode the index is set. This is most likely
	 * called after the sorting criteria have been changed. */
	int i = 1;
	if (initial)
	{
		for (CoreAttributes* c = first(); c != 0; c = next(), ++i)
			c->setSequenceNo(i);
	}
	else
	{
		sort();
		for (CoreAttributes* c = first(); c != 0; c = next(), ++i)
			c->setIndex(i);
	}
}

uint
CoreAttributesList::maxDepth()
{
	uint md = 0;
	for (CoreAttributes* c = first(); c != 0; c = next())
		if (c->treeLevel() + 1 > md)
			md = c->treeLevel() + 1;
	return md;
}

bool
CoreAttributesList::isSupportedSortingCriteria
	(CoreAttributesList::SortCriteria sc)
{
	switch (sc)
	{
	case Sequence:
	case TreeMode:
	case FullNameDown:
	case FullNameUp:
	case IndexUp:
	case IndexDown:
	case IdUp:
	case NameUp:
	case NameDown:
		return TRUE;
	default:
		return FALSE;
	}		
}

int
CoreAttributesList::compareItemsLevel(CoreAttributes* c1, CoreAttributes* c2,
									  int level)
{
	if (level < 0 || level >= maxSortingLevel)
		return -1;
	
	switch (sorting[level])
	{
	case Sequence:
		return c1->getSequenceNo() == c2->getSequenceNo() ? 0 :
			c1->getSequenceNo() < c2->getSequenceNo() ? -1 : 1;
	case TreeMode:
	{
		if (level == 0)
			return compareTreeItemsT(this, c1, c2);
		else
			return c1->getSequenceNo() < c2->getSequenceNo() ? -1 : 1;
	}
	case FullNameDown:
	{
		QString fn1;
		c1->getFullName(fn1);
		QString fn2;
		c2->getFullName(fn2);
		return fn1.compare(fn2);
	}
	case FullNameUp:
	{
		QString fn1;
		c1->getFullName(fn1);
		QString fn2;
		c2->getFullName(fn2);
		return fn2.compare(fn1);
	}
	case IndexUp:
		return c2->getIndex() == c1->getIndex() ? 0 :
			c2->getIndex() < c1->getIndex() ? -1 : 1;
	case IndexDown:
		return c1->getIndex() == c2->getIndex() ? 0:
			c1->getIndex() > c2->getIndex() ? -1 : 1;
	case IdUp:
		return QString::compare(c1->getId(), c2->getId());
	case IdDown:
		return QString::compare(c2->getId(), c1->getId());
	case NameUp:
		return c1->getName().compare(c2->getName());
	case NameDown:
		return c2->getName().compare(c1->getName());
	default:
		qFatal("CoreAttributesList:compareItemsLevel: "
			   "Please implement sorting for mode (%d/%d) in sub class!",
			   sorting[level], level);
	}
	return 0;
}

int
CoreAttributesList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	CoreAttributes* c1 = static_cast<CoreAttributes*>(i1);
	CoreAttributes* c2 = static_cast<CoreAttributes*>(i2);

	int res;
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
		if ((res = compareItemsLevel(c1, c2, i)) != 0)
			return res;
	return res;
}

uint
CoreAttributes::treeLevel() const
{
	uint tl = 0;
	for (CoreAttributes* c = parent; c; c = c->parent)
		tl++;
	return tl;
}

void
CoreAttributes::getFullName(QString& fullName) const
{
	fullName = "";
	for (const CoreAttributes* c = this; c != 0; c = c->parent)
		fullName = c->name + "." + fullName;
	// Remove trailing dot.
	fullName.remove(fullName.length() - 1, 1);
}

QString
CoreAttributes::getFullId() const
{
	QString fullID = id;
	for (const CoreAttributes* c = parent; c != 0; c = c->parent)
		fullID = c->id + "." + fullID;
	return fullID;
}
