/*
 * QtReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include "QtReport.h"
#include "Project.h"
#include "Utility.h"

QtReport::QtReport(Project* p, const QString& f, const QString& df,
                       int dl) :
   Report(p, f, df, dl)
{
    loadUnit = shortAuto;
}

void
QtReport::generateHeader()
{
}

void
QtReport::generateFooter()
{
}

