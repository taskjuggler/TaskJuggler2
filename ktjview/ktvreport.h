/*
 * ktvreport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_ktv_h_
#define _Report_ktv_h_


#include "Report.h"

class Project;

class KTVReport: public Report
{
public:
   KTVReport(Project* p, const QString& f, time_t s, time_t e);
   virtual ~KTVReport() { }

} ;

#endif

