/*
 * Journal.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */
#include "Journal.h"

int
Journal::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
    return (static_cast<JournalEntry*>(item1))->getDate() -
        (static_cast<JournalEntry*>(item2))->getDate();
}

