/*
 * Journal.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Journal_h_
#define _Journal_h_

#include <qptrlist.h>

#include "JournalEntry.h"

class Journal : public QPtrList<JournalEntry>
{
public:
    Journal()
    {
        setAutoDelete(true);
    }
    ~Journal() { }

    typedef QPtrListIterator<JournalEntry> Iterator;

protected:
    virtual int compareItems(QPtrCollection::Item item1,
                             QPtrCollection::Item item2);
} ;

#endif

