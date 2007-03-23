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

bool
HTMLWeeklyCalendar::generate()
{
    if (!open())
        return false;

    generateHeader(i18n("Weekly Calendar"));
    generateBody();
    generateFooter();

    f.close();
    return true;
}
