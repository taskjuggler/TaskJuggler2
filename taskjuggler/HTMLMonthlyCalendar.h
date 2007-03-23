/*
 * HTMLMonthlyCalendar.h - TaskJuggler
 *
 * Copyright (c) 2006 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: HTMLMonthlyCalendar.h 1259 2006-01-31 12:04:00Z cs $
 */

#ifndef _HTMLMonthlyCalendar_h_
#define _HTMLMonthlyCalendar_h_

#include "HTMLSingleReport.h"
#include "HTMLMonthlyCalendarElement.h"

class HTMLMonthlyCalendar : public HTMLSingleReport
{
public:
    HTMLMonthlyCalendar(Project* p, const QString& f, const QString& df, int dl) :
        HTMLSingleReport(p, f, df, dl)
    {
        setTable(new HTMLMonthlyCalendarElement(this, df, dl));
    }

    virtual ~HTMLMonthlyCalendar()
    { }

    virtual const char* getType() const { return "HTMLMonthlyCalendar"; }

    bool generate();
} ;

#endif

