/*
 * HTMLWeeklyCalendar.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLWeeklyCalendar_h_
#define _HTMLWeeklyCalendar_h_

#include "HTMLReport.h"

class Project;
class HTMLWeeklyCalendarElement;

class HTMLWeeklyCalendar : public HTMLReport
{
public:
    HTMLWeeklyCalendar(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLWeeklyCalendar();

    bool generate();
    HTMLWeeklyCalendarElement* getTable() { return tab; }

private:
    HTMLWeeklyCalendar() { }    // don't call this directly

    HTMLWeeklyCalendarElement* tab;
} ;

#endif

