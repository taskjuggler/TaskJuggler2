/*
 * VacationList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _VacationList_h_
#define _VacationList_h_

#include "time.h"

#include <qptrlist.h>
#include <qstring.h>

class Interval;
class VacationInterval;

/**
 * @short A list of vacations.
 * @author Chris Schlaeger <cs@suse.de>
 */
class VacationList : public QPtrList<VacationInterval>
{
public:
    VacationList() { setAutoDelete(TRUE); }
    virtual ~VacationList() {}

    void add(const QString& name, const Interval& i);
    void add(VacationInterval* vi);
    bool isVacation(time_t date) const;

protected:
    virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator for VacationList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class VacationListIterator : public QPtrListIterator<VacationInterval>
{
public:
    VacationListIterator(const VacationList& l) :
        QPtrListIterator<VacationInterval>(l) { }
    virtual ~VacationListIterator() { }
} ;

#endif
