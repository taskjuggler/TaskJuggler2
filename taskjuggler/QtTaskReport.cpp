/*
 * QtTaskReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <qfile.h>

#include "QtTaskReport.h"
#include "QtTaskReportElement.h"

QtTaskReport::QtTaskReport(Project* p, const QString& f, const QString& df,
                               int dl) :
    QtReport(p, f, df, dl)
{
    tab = new QtTaskReportElement(this, df, dl);
}

QtTaskReport::~QtTaskReport()
{
    delete tab;
}

bool
QtTaskReport::generate()
{
    generateHeader();
    tab->generate();
    generateFooter();

    return TRUE;
}
