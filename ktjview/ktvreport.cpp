/*
 * ktvreport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *                             Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#include "Report.h"
#include "ktvreport.h"

KTVReport::KTVReport( Project *p, const QString& f, time_t s, time_t e) :
   Report(p, f, s, e, QString(), 0 )
{
   
}
