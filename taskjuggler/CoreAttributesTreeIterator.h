/*
 * CoreAttributesTreeIterator.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CoreAttributesTreeIterator_h_
#define _CoreAttributesTreeIterator_h_

class CoreAttributes;

class CoreAttributesTreeIterator
{
public:
	CoreAttributesTreeIterator(CoreAttributes* root);
	~CoreAttributesTreeIterator() { }

	CoreAttributes* operator*() { return current; }
	CoreAttributes* operator++();

protected:
	CoreAttributes* current;
private:
	CoreAttributes* root;
} ;

#endif

