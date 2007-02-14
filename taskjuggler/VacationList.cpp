/*
 * VacationList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "VacationList.h"
#include "VacationInterval.h"

void
VacationList::add(const QString& name, const Interval& i)
{
    inSort(new VacationInterval(name, i));
}

void
VacationList::add(VacationInterval* vi)
{
    inSort(vi);
}

int
VacationList::compareItems(QCollection::Item it1, QCollection::Item it2)
{
    Interval* i1 = static_cast<Interval*>(it1);
    Interval* i2 = static_cast<Interval*>(it2);

    if (i1->getStart() == i2->getStart())
    {
        if (i1->getEnd() == i2->getEnd())
            return 0;
        else
            return i2->getEnd() - i1->getEnd();
    }
    else
        return i2->getStart() - i1->getStart();
}

bool
VacationList::isVacation(time_t date) const
{
    for (VacationList::Iterator vli(*this); *vli != 0; ++vli)
        if ((*vli)->contains(date))
            return TRUE;

    return FALSE;
}

const QString&
VacationList::vacationName(time_t date) const
{
    for (VacationList::Iterator vli(*this); *vli != 0; ++vli)
        if ((*vli)->contains(date))
            return (*vli)->getName();

    return QString::null;
}
