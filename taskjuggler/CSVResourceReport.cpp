/*
 * CSVResourceReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <qfile.h>

#include "CSVResourceReport.h"
#include "CSVResourceReportElement.h"

CSVResourceReport::CSVResourceReport(Project* p, const QString& f,
                                       const QString& df, int dl) :
    CSVReport(p, f, df, dl)
{
    tab = new CSVResourceReportElement(this, df, dl);
}

bool
CSVResourceReport::generate()
{
    if (!open())
        return FALSE;

    generateHeader();
    tab->generate();
    generateFooter();

    f.close();
    return TRUE;
}
