/*
 * CSVTaskReport.cpp - TaskJuggler
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

#include "CSVTaskReport.h"
#include "CSVTaskReportElement.h"

CSVTaskReport::CSVTaskReport(Project* p, const QString& f, const QString& df, 
                               int dl) :
    CSVReport(p, f, df, dl)
{
    tab = new CSVTaskReportElement(this, df, dl);
}

CSVTaskReport::~CSVTaskReport()
{
    delete tab;
}

bool
CSVTaskReport::generate()
{
    if (!open())
        return FALSE;

    generateHeader();
    tab->generate();
    generateFooter();

    f.close();
    return TRUE;
}
