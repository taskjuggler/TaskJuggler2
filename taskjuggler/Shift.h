/*
 * Shift.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Shift_h_
#define _Shift_h_

#include <time.h>

#include <qptrlist.h>

#include "CoreAttributes.h"

class Interval;

/**
 * @short Stores all shift related information.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Shift : public CoreAttributes
{
public:
    Shift(Project* prj, const QString& i, const QString& n, Shift* p);
    virtual ~Shift();

    virtual const char* getType() const { return "Shift"; }

    Shift* getParent() const { return (Shift*) parent; }

    void setWorkingHours(int day, QPtrList<Interval>* l)
    {
        if (day < 0 || day > 6)
            qFatal("day out of range");
        delete workingHours[day];
        workingHours[day] = l;
    }

    QPtrList<Interval>* getWorkingHours(int day) const
    {
        if (day < 0 || day > 6)
            qFatal("day out of range");
        return workingHours[day];
    }

    bool isOnShift(const Interval& iv) const;

    bool isVacationDay(time_t day) const;

private:
    Shift() { }     // Don't use this.
    
    QPtrList<Interval>* workingHours[7];    
};

#endif

