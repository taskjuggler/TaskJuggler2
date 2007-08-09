/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjObjPosTable_h_
#define _TjObjPosTable_h_

#include <map>

#include <ltQString.h>

#include "TjObjPosTableEntry.h"

class CoreAttributes;

class TjObjPosTable
{
    friend class TjObjPosTableConstIterator;

public:
    TjObjPosTable() { }
    ~TjObjPosTable();

    TjObjPosTableEntry* addEntry(CoreAttributes* ca, CoreAttributes* subCa,
                                 int pos, int height);

    int caToPos(const CoreAttributes* ca, const CoreAttributes* subCa = 0)
        const;

    int caToHeight(const CoreAttributes* ca, const CoreAttributes* subCa = 0)
        const;

    bool isEmpty() const { return entries.empty(); }

private:
    QString generateKey(const CoreAttributes* ca,
                        const CoreAttributes* subCa) const;

    std::map<const QString, TjObjPosTableEntry*, ltQString> entries;
} ;

class TjObjPosTableConstIterator
{
public:
    TjObjPosTableConstIterator(const TjObjPosTable& o) : opt(o)
    {
        it = opt.entries.begin();
    }
    ~TjObjPosTableConstIterator() { }

    TjObjPosTableEntry* operator*() const
    {
        return it == opt.entries.end() ? 0 : (*it).second;
    }
    void operator++() { ++it; }

private:
    const TjObjPosTable& opt;
    std::map<const QString, TjObjPosTableEntry*, ltQString>::const_iterator it;
} ;

#endif

