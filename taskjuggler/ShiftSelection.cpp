/*
 * ShiftSelection.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#include "ShiftSelection.h"

bool
ShiftSelection::isVacationDay(time_t day) const
{
	return period.contains(day) && shift->isVacationDay(day);
}


