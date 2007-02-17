/*
 * HTMLMonthlyCalendar.cpp - TaskJuggler
 *
 * Copyright (c) 2006 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: HTMLMonthlyCalendar.cpp 1259 2006-01-31 12:04:00Z cs $
 */

#include <qfile.h>

#include "tjlib-internal.h"
#include "HTMLMonthlyCalendar.h"
#include "HTMLMonthlyCalendarElement.h"

HTMLMonthlyCalendar::HTMLMonthlyCalendar(Project* p, const QString& f,
                                       const QString& df, int dl) :
    HTMLReport(p, f, df, dl),
    tab(new HTMLMonthlyCalendarElement(this, df, dl))
{
}

HTMLMonthlyCalendar::~HTMLMonthlyCalendar()
{
    delete tab;
}

bool
HTMLMonthlyCalendar::generate()
{
    if (!open())
        return FALSE;

    generateHeader(i18n("Weekly Calendar"));
    tab->generate();
    generateFooter();

    f.close();
    return TRUE;
}

