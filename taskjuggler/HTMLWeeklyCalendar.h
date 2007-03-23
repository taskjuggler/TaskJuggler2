/*
 * HTMLWeeklyCalendar.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLWeeklyCalendar_h_
#define _HTMLWeeklyCalendar_h_

#include "HTMLSingleReport.h"
#include "HTMLWeeklyCalendarElement.h"

class HTMLWeeklyCalendar : public HTMLSingleReport
{
public:
    HTMLWeeklyCalendar(Project* p, const QString& f, const QString& df, int dl) :
        HTMLSingleReport(p, f, df, dl)
    {
        setTable(new HTMLWeeklyCalendarElement(this, df, dl));
    }

    virtual ~HTMLWeeklyCalendar()
    { }

    virtual const char* getType() const { return "HTMLWeeklyCalendar"; }

    bool generate();
};

#endif

