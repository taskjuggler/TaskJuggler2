/*
 * HTMLTaskReport.cpp - TaskJuggler
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

#include "HTMLTaskReport.h"
#include "HTMLTaskReportElement.h"

HTMLTaskReport::HTMLTaskReport(Project* p, const QString& f, const QString& df,
                               int dl) :
    HTMLReport(p, f, df, dl)
{
    tab = new HTMLTaskReportElement(this, df, dl);
}

HTMLTaskReport::~HTMLTaskReport()
{
    delete tab;
}

bool
HTMLTaskReport::generate()
{
    if (!open())
        return FALSE;

    generateHeader("Task Report");
    tab->generate();
    generateFooter();

    f.close();
    return TRUE;
}
