/*
 * BookingList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _BookingList_h_
#define _BookingList_h_

#include <qptrlist.h>

#include "Booking.h"

/**
 * @short A list of bookings.
 * @author Chris Schlaeger <cs@suse.de>
 */
class BookingList : public QPtrList<Booking>
{
public:
    BookingList() { }
    virtual ~BookingList() { }

protected:
    virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
};

/**
 * @short Iterator class for BookingList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class BookingListIterator : public QPtrListIterator<Booking> 
{
public:
    BookingListIterator(const BookingList& l) :
        QPtrListIterator<Booking>(l) { }
    virtual ~BookingListIterator() { }
} ;

#endif

