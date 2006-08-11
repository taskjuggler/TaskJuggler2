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

#include "TjObjPosTable.h"

#include <assert.h>

#include "CoreAttributes.h"

TjObjPosTable::~TjObjPosTable()
{
    for (TjObjPosTableConstIterator it(*this); *it != 0; ++it)
        delete *it;
    entries.clear();
}

void
TjObjPosTable::addEntry(CoreAttributes* ca, CoreAttributes* subCa,
                        int pos, int height)
{
    TjObjPosTableEntry* entry = new TjObjPosTableEntry(ca, subCa, pos, height);
    entries[generateKey(ca, subCa)] = entry;
}

int
TjObjPosTable::caToPos(const CoreAttributes* ca, const CoreAttributes* subCa)
    const
{
    assert(ca);

    QString key = generateKey(ca, subCa);

    return entries.find(key) == entries.end() ? -1 :
        (*entries.find(key)).second->getPos();
}

int
TjObjPosTable::caToHeight(const CoreAttributes* ca,
                          const CoreAttributes* subCa) const
{
    assert(ca);

    QString key = generateKey(ca, subCa);

    return entries.find(key) == entries.end() ? -1 :
        (*entries.find(key)).second->getHeight();
}

QString
TjObjPosTable::generateKey(const CoreAttributes* ca,
                           const CoreAttributes* subCa) const
{
    QString key;
    if (ca->getType() == CA_Task)
    {
        if (subCa)
            key = QString("r:") + ca->getFullId() + ":" + subCa->getId();
        else
            key = QString("t:") + ca->getFullId();
    }
    else
    {
        if (subCa)
            key = QString("t:") + ca->getId() + ":" + subCa->getFullId();
        else
            key = QString("r:") + ca->getId();
    }

    return key;
}
