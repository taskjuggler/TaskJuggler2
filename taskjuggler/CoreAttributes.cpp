/*
 * CoreAttributes.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CoreAttributes.h"

CoreAttributes::~CoreAttributes()
{
    while (!sub.isEmpty())
        delete sub.getFirst();
    if (parent)
        parent->sub.removeRef(this);
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
	fullName = QString::null;
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

bool
CoreAttributes::hasSameAncestor(CoreAttributes* c) const
{
	if (c == 0)
		return FALSE;
	
	CoreAttributes const* p1;
	for (p1 = this; p1->parent; p1 = p1->parent)
		;
	CoreAttributes const* p2;
	for (p2 = c; p2->parent; p2 = p2->parent)
		;
	return p1 == p2;
}

bool
CoreAttributes::isDescendentOf(CoreAttributes* c) const
{
	if (c == 0)
		return FALSE;

	for (CoreAttributes const* p = this; p; p = p->parent)
		if (p == c)
			return TRUE;
	return FALSE;
}
