/*
 * VacationInterval.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _VacationInterval_h_
#define _VacationInterval_h_

#include <qstring.h>

#include "Interval.h"

/**
 * @short An interval with a name.
 * @author Chris Schlaeger <cs@suse.de>
 */
class VacationInterval : public Interval
{
public:
    VacationInterval() { }

    VacationInterval(const QString& n, const Interval& i)
        : Interval(i), name(n) { }
    virtual ~VacationInterval() { }

    const QString& getName() const { return name; }

private:
    QString name;
} ;

#endif

