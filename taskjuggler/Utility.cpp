/*
 * Utility.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Utility.h"

QString time2ISO(time_t t)
{
	struct tm* tms = localtime(&t);
	static QString s;
	s.sprintf("%04d-%02d-%02d", 1900 + tms->tm_year, 1 + tms->tm_mon,
			  tms->tm_mday);
	return s;
}

