/*
 * ResourceTreeIterator.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ResourceTreeIterator_h_
#define _ResourceTreeIterator_h_

#include "CoreAttributesTreeIterator.h"

class ResourceTreeIterator : public virtual CoreAttributesTreeIterator
{
public:
    ResourceTreeIterator(Resource* r,
                         CoreAttributesTreeIterator::IterationMode m =
                         CoreAttributesTreeIterator::leavesOnly) 
        : CoreAttributesTreeIterator(r, m) { } 
    virtual ~ResourceTreeIterator() { }

    Resource* operator*() { return (Resource*) current; }
    Resource* operator++() 
    {
        return (Resource*) CoreAttributesTreeIterator::operator++();
    }
} ;

#endif

