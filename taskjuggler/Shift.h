/*
 * Shift.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * by Chris Schlaeger <cs@kde.org>
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
class ShiftListIterator;

/**
 * @short Stores all shift related information.
 * @author Chris Schlaeger <cs@kde.org>
 */
class Shift : public CoreAttributes
{
public:
    Shift(Project* prj, const QString& i, const QString& n, Shift* p,
          const QString& df = QString::null, uint dl = 0);
    virtual ~Shift();

    virtual CAType getType() const { return CA_Shift; }

    Shift* getParent() const { return (Shift*) parent; }

    ShiftListIterator getSubListIterator() const;

    void inheritValues();

    void setWorkingHours(int day, const QPtrList<Interval>& l);

    QPtrList<Interval>* getWorkingHours(int day) const
    {
        return workingHours[day];
    }
    const QPtrList<Interval>* const * getWorkingHours() const
    {
        return static_cast<const QPtrList<Interval>* const*>(workingHours);
    }

    bool isOnShift(const Interval& iv) const;

    bool isVacationDay(time_t day) const;

private:
    Shift(); // leave unimplemented

    QPtrList<Interval>* workingHours[7];
};

#endif

