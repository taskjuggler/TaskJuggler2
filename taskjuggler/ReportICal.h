/*
 * Report.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_ical_h_
#define _Report_ical_h_
#ifdef HAVE_ICAL

#include <stdio.h>
#include <time.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qtextstream.h>
#ifdef HAVE_KDE
#include <libkcal/todo.h>
#include <libkcal/icalformat.h>
#endif

#include "Report.h"

class Project;
class ExpressionTree;

class ReportICal: public Report
{
public:
   ReportICal(Project* p, const QString& f, time_t s, time_t e);
   virtual ~ReportICal() { }

   void generate();
   
protected:
   KCal::Todo* addATask( Task *task, KCal::CalendarLocal *cal );
   ReportICal() { }
} ;

#endif
#endif
