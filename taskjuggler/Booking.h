/*
 * Booking.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Booking_h_
#define _Booking_h_

#include "Interval.h"
#include "SbBooking.h"

/**
 * @short Booking information for an interval of the resource.
 * @author Chris Schlaeger <cs@kde.org>
 */
class Booking : public SbBooking
{
public:
    Booking(const Interval& iv, Task* t) :
        SbBooking(t),
        interval(new Interval(iv))
    { }

    Booking(Interval* iv, Task* t) :
        SbBooking(t),
        interval(iv)
    { }

    Booking(const Interval& iv, SbBooking* sb) :
        SbBooking(*sb),
        interval(new Interval(iv))
    { }

    virtual ~Booking() { delete interval; }

    time_t getStart() const { return interval->getStart(); }
    time_t getEnd() const { return interval->getEnd(); }
    time_t getDuration() const { return interval->getDuration(); }
    Interval& getInterval() { return *interval; }

private:
    /// The booked time period.
    Interval* interval;
} ;

#endif

