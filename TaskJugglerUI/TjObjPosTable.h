/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjReport.cpp 1136 2005-08-13 02:49:59Z cs $
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

    void addEntry(const CoreAttributes* ca, const CoreAttributes* subCa,
                  int pos, int height);

    int caToPos(const CoreAttributes* ca, const CoreAttributes* subCa = 0)
        const;

    int caToHeight(const CoreAttributes* ca, const CoreAttributes* subCa = 0)
        const;

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

