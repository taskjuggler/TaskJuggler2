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
CoreAttributesList::setSorting(SortCriteria s)
{
	sorting = s;
}

void
CoreAttributesList::createIndex()
{
	int i = 1;
	for (CoreAttributes* c = first(); c != 0; c = next(), ++i)
		c->setSequenceNo(i);

	SortCriteria savedSorting = sorting;
	sorting = TreeMode;
	sort();
	i = 1;
	for (CoreAttributes* c = first(); c != 0; c = next(), ++i)
		c->setIndex(i);
	sorting = savedSorting;
	sort();
}

int
CoreAttributesList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	CoreAttributes* c1 = static_cast<CoreAttributes*>(i1);
	CoreAttributes* c2 = static_cast<CoreAttributes*>(i2);

	switch (sorting)
	{
	case Pointer:
		return c1 - c2;
	case TreeMode:
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
