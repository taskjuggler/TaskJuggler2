/*
 * ShiftList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ShiftList.h"
#include "Shift.h"
#include "Project.h"

Shift*
ShiftList::getShift(const QString& id) const
{
    for (ShiftListIterator sli(*this); *sli != 0; ++sli)
        if ((*sli)->getId() == id)
            return *sli;

    return 0;
}

int
ShiftList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
    Shift* s1 = static_cast<Shift*>(i1);
    Shift* s2 = static_cast<Shift*>(i2);

    int res;
    for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
        if ((res = compareItemsLevel(s1, s2, i)) != 0)
            return res;
    return res;
}

int 
ShiftList::compareItemsLevel(Shift* s1, Shift *s2,
                                      int level)
{
    if (level < 0 || level >= maxSortingLevel)
        return -1;

    switch (sorting[level])
    {
    case TreeMode:
        if (level == 0)
            return compareTreeItemsT(this, s1, s2);
        else
            return s1->getSequenceNo() == s2->getSequenceNo() ? 0 :
                s1->getSequenceNo() < s2->getSequenceNo() ? -1 : 1;
    default:
        return CoreAttributesList::compareItemsLevel(s1, s2, level);
    }       
}

bool
Shift::isOnShift(const Interval& iv) const
{
    int dow = dayOfWeek(iv.getStart(), FALSE);
    for (QPtrListIterator<Interval> ili(*(workingHours[dow])); *ili != 0; ++ili)
    {
        if ((*ili)->contains(Interval(secondsOfDay(iv.getStart()),
                                  secondsOfDay(iv.getEnd()))))
            return TRUE;
    }
    return FALSE;
}

