/*
 * HTMLAccountReport.h - TaskJuggler
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

#include "HTMLAccountReport.h"
#include "HTMLAccountReportElement.h"

#define KW(a) a

HTMLAccountReport::HTMLAccountReport(Project* p, const QString& f, 
                                     const QString& df, int dl) :
    HTMLReport(p, f, df, dl)
{
    tab = new HTMLAccountReportElement(this, df, dl);
}

HTMLAccountReport::~HTMLAccountReport()
{
    delete tab;
}

bool
HTMLAccountReport::generate()
{
    if (!open())
        return FALSE;

    generateHeader();
    tab->generate();
    generateFooter();

    f.close();
    return TRUE;
}

