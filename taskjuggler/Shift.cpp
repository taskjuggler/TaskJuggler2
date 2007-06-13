/*
 * Shift.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Shift.h"
#include "Project.h"

Shift::Shift(Project* prj, const QString& i, const QString& n, Shift* p,
             const QString& df, uint dl) :
    CoreAttributes(prj, i, n, p, df, dl),
    workingHours()
{
    prj->addShift(this);

    for (int i = 0; i < 7; i++)
    {
        workingHours[i] = new QPtrList<Interval>();
        workingHours[i]->setAutoDelete(true);
    }
}

Shift::~Shift()
{
    for (int i = 0; i < 7; i++)
        delete workingHours[i];
    project->deleteShift(this);
}

void
Shift::inheritValues()
{
    Shift* p = (Shift*) parent;

    if (p)
    {
        // Inherit start values from parent resource.
        for (int i = 0; i < 7; i++)
        {
            delete workingHours[i];
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(true);
            for (QPtrListIterator<Interval> ivi(*(p->workingHours[i]));
                 *ivi != 0; ++ivi)
                workingHours[i]->append(new Interval(**ivi));
        }
    }
    else
    {
        // Inherit start values from project defaults.
        for (int i = 0; i < 7; i++)
        {
            delete workingHours[i];
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(true);
            for (QPtrListIterator<Interval>
                 ivi(project->getWorkingHoursIterator(i));
                 *ivi != 0; ++ivi)
                workingHours[i]->append(new Interval(**ivi));
        }
    }
}

void
Shift::setWorkingHours(int day, const QPtrList<Interval>& l)
{
    delete workingHours[day];

    // Create a deep copy of the interval list.
    workingHours[day] = new QPtrList<Interval>;
    workingHours[day]->setAutoDelete(true);
    for (QPtrListIterator<Interval> pli(l); pli; ++pli)
        workingHours[day]->append(new Interval(**pli));
}

ShiftListIterator
Shift::getSubListIterator() const
{
    return ShiftListIterator(*sub);
}

bool
Shift::isOnShift(const Interval& iv) const
{
    int dow = dayOfWeek(iv.getStart(), false);
    int ivStart = secondsOfDay(iv.getStart());
    int ivEnd = secondsOfDay(iv.getEnd());
    Interval dayIv(ivStart, ivEnd);
    for (QPtrListIterator<Interval> ili(*(workingHours[dow])); *ili != 0; ++ili)
        if ((*ili)->contains(dayIv))
            return true;

    return false;
}

bool
Shift::isVacationDay(time_t day) const
{
    return workingHours[dayOfWeek(day, false)]->isEmpty();
}

