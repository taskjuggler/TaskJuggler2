/*
 * Booking.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Booking_h_
#define _Booking_h_

#include <qstring.h>

#include "Interval.h"
#include "SbBooking.h"

/**
 * @short Booking information for an interval of the resource.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Booking : public SbBooking
{
public:
	Booking(const Interval& iv, Task* t, QString a = "",
			QString i = "")
		: SbBooking(t, a, i), interval(iv) { }
	Booking(const Interval& iv, SbBooking* sb) : SbBooking(*sb),
			interval(iv) { }
	~Booking() { }

	time_t getStart() const { return interval.getStart(); }
	time_t getEnd() const { return interval.getEnd(); }
	time_t getDuration() const { return interval.getDuration(); }
	Interval& getInterval() { return interval; }

	void setLockTS( const QString& ts ) { lockTS = ts; }
	const QString& getLockTS() const { return lockTS; }

	void setLockerId( const QString& id ) { lockerId = id; }
	const QString& getLockerId() const { return lockerId; }

private:
	/// The booked time period.
	Interval interval;
	/// The database lock timestamp
	QString lockTS;

	/// the lockers ID
	QString lockerId;
} ;

#endif

