/*
 * JournalEntry.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _JournalEntry_h_
#define _JournalEntry_h_

#include <time.h>

#include <qstring.h>

class JournalEntry
{
public:
    JournalEntry(time_t d, const QString& s) : date(d), text(s) { }
    ~JournalEntry() { }

    time_t getDate() const { return date; }
    const QString& getText() const { return text; }

private:
    time_t date;
    QString text;
} ;

#endif

