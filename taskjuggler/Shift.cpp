/*
 * Shift.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Shift.h"
#include "Interval.h"
#include "Project.h"

Shift::Shift(Project* prj, const QString& i, const QString& n, Shift* p) :
    CoreAttributes(prj, i, n, p)
{
    prj->addShift(this);

    if (p)
    {
        // Inherit start values from parent resource.
        for (int i = 0; i < 7; i++)
        {
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(TRUE);
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
            workingHours[i] = new QPtrList<Interval>();
            workingHours[i]->setAutoDelete(TRUE);
            for (QPtrListIterator<Interval>
                 ivi(prj->getWorkingHoursIterator(i));
                 *ivi != 0; ++ivi)
                workingHours[i]->append(new Interval(**ivi));
        }
    }
}

Shift::~Shift()
{
    for (int i = 0; i < 7; i++)
        delete workingHours[i];
    project->deleteShift(this);
}

ShiftListIterator
Shift::getSubListIterator() const
{
    return ShiftListIterator(sub);
}

bool 
Shift::isVacationDay(time_t day) const
{
    return workingHours[dayOfWeek(day, FALSE)]->isEmpty();
}

