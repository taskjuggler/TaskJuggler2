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

#include "TjObjPosTable.h"

#include <assert.h>

#include "CoreAttributes.h"

void
TjObjPosTable::resize(int sz)
{
    static int primes[] = {
        17, 73, 157, 239, 331, 421, 509, 613, 709, 821, 919, 1019, 1093, 1187,
        1279, 1367, 1453, 1543, 1613, 1709, 1801, 1901, 1999, 2087, 2179, 2281,
        2371, 2447, 2557, 2671, 2731, 2833, 2927, 3037, 3163, 3253, 3343, 3457,
        3539, 3623, 3719, 3823, 3919, 4019, 4127, 4229, 4327, 4441, 4523, 4643,
        4733, 4861, 4957, 5039, 5153, 5273, 5393, 5477, 5569, 5669, 5783, 5861,
        5987, 6091, 6203, 6299, 6373, 6521, 6619, 6719, 6829, 6947, 7019, 7151,
        7247, 7393, 7507, 7583, 7687, 7793, 7907, 8039, 8147, 8243, 8363, 8467,
        8599, 8693, 8783, 8887, 9007, 9127, 9221, 9337, 9431, 9521, 9643, 9749,
        9851, 9949
    };

    // Find a prime number that is slightly larger than the resquested size.
    uint i;
    for (i = 0; i < sizeof(primes) / sizeof(int); i++)
        if (sz < primes[i])
            break;

    entries.resize(primes[i]);
}

void
TjObjPosTable::addEntry(const CoreAttributes* ca, const CoreAttributes* subCa,
                        int pos, int height)
{
    TjObjPosTableEntry* entry = new TjObjPosTableEntry(ca, subCa, pos, height);
    if (subCa)
        entries.insert(ca->getFullId() + ":" + subCa->getFullId(), entry);
    else
        entries.insert(ca->getFullId(), entry);
}

int
TjObjPosTable::caToPos(const CoreAttributes* ca, const CoreAttributes* subCa)
    const
{
    assert(ca);

    QString key;
    if (subCa)
        key = ca->getFullId() + ":" + subCa->getFullId();
    else
        key = ca->getFullId();

    return entries[key] == 0 ? -1 : entries[key]->getPos();
}

int
TjObjPosTable::caToHeight(const CoreAttributes* ca,
                          const CoreAttributes* subCa) const
{
    assert(ca);

    QString key;

    if (subCa)
        key = ca->getFullId() + ":" + subCa->getFullId();
    else
        key = ca->getFullId();

    return entries[key] == 0 ? -1 : entries[key]->getHeight();
}

