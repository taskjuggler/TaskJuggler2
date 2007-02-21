/*
 * HTMLWeeklyCalendar.cpp - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLWeeklyCalendar.h"

#include <qfile.h>

#include "tjlib-internal.h"

HTMLWeeklyCalendar::HTMLWeeklyCalendar(Project* p, const QString& f,
                                       const QString& df, int dl) :
    HTMLReport(p, f, df, dl),
    tab(this, df, dl)
{
}

HTMLWeeklyCalendar::~HTMLWeeklyCalendar()
{
}

bool
HTMLWeeklyCalendar::generate()
{
    if (!open())
        return FALSE;

    generateHeader(i18n("Weekly Calendar"));
    tab.generate();
    generateFooter();

    f.close();
    return TRUE;
}
