/*
 * CoreAttributesTreeIterator - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CoreAttributesTreeIterator.h"
#include "CoreAttributes.h"

CoreAttributesTreeIterator::CoreAttributesTreeIterator(CoreAttributes* r)
{
	root = current = r;
	while (current->hasSubs())
		current = current->getSubList().getFirst();
}

CoreAttributes*
CoreAttributesTreeIterator::operator++()
{
	if (current == 0)
		return 0;

	while (current != root)
	{
		// Find the current CA in the parent's sub list.
		CoreAttributesListIterator
			cli(current->getParent()->getSubListIterator());
		for ( ; *cli != current; ++cli)
			;
		// Check if there is another task in the sub list.
		++cli;
		if (*cli != 0)
		{
			// Find the first leaf in this sub list.
			current = *cli;
			while (current->hasSubs())
				current = current->getSubList().getFirst();
			// This is the new current task.
			return current;
		}
		// End of sub list reached. Try parent node then.
		current = current->getParent();
	}
	return (current = 0);
}

