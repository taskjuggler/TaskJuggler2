/*
 * ResourceList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ResourceList_h_
#define _ResourceList_h_

#include "CoreAttributesList.h"

class QString;
class Resource;

/**
 * @short A list of resources.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ResourceList : public CoreAttributesList
{
public:
	ResourceList();
	virtual ~ResourceList() { }

	Resource* getResource(const QString& id) const;

	static bool isSupportedSortingCriteria(int sc);
	
	virtual int compareItemsLevel(Resource* r1, Resource* r2, int level);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator class for ResourceList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ResourceListIterator : public virtual CoreAttributesListIterator 
{
public:
	ResourceListIterator(const CoreAttributesList& l) :
		CoreAttributesListIterator(l) { }
	~ResourceListIterator() { }
	Resource* operator*() { return (Resource*) get(); }
} ;

#endif
