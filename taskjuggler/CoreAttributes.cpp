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
	sorting[level] = s;
}

void
CoreAttributesList::createIndex()
{
	int i = 1;
	for (CoreAttributes* c = first(); c != 0; c = next(), ++i)
		c->setSequenceNo(i);

	SortCriteria savedSorting = sorting[0];
	sorting[0] = TreeMode;
	sort();
	i = 1;
	for (CoreAttributes* c = first(); c != 0; c = next(), ++i)
		c->setIndex(i);
	sorting[0] = savedSorting;
	sort();
}

int
CoreAttributesList::compareItemsLevel(CoreAttributes* c1,
									  CoreAttributes* c2, int level)
{
	if (level > 2)
		return -1;
	
	switch (sorting[level])
	{
	case Sequence:
			return c2->getSequenceNo() - c1->getSequenceNo();
	case TreeMode:
	case FullNameDown:
	{
		QString fn1;
		c1->getFullName(fn1);
		QString fn2;
		c2->getFullName(fn2);
		if (fn1.compare(fn2) == 0)
			return this->compareItemsLevel(c1, c2, level + 1);
		else
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
		return c2->getIndex() - c1->getIndex();
	case IndexDown:
		return c1->getIndex() - c2->getIndex();
	case IdUp:
		return QString::compare(c1->getId(), c2->getId());
	case IdDown:
		return QString::compare(c2->getId(), c1->getId());
	case NameUp:
		return c1->getName().compare(c2->getName());
	case NameDown:
		return c2->getName().compare(c1->getName());
	default:
		qFatal("Please implement sorting for mode %d in sub class!", sorting);
	}
	return 0;
	
}

int
CoreAttributesList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	CoreAttributes* c1 = static_cast<CoreAttributes*>(i1);
	CoreAttributes* c2 = static_cast<CoreAttributes*>(i2);

	compareItemsLevel(c1, c2, 0);
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
