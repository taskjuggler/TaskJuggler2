/*
 * HTMLResourceReport.cpp - TaskJuggler
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

#include "HTMLResourceReport.h"
//#include "Project.h"
//#include "ResourceList.h"
#include "HTMLResourceReportElement.h"
//#include "ExpressionTree.h"
//#include "Operation.h"

HTMLResourceReport::HTMLResourceReport(Project* p, const QString& f,
                                       const QString& df, int dl) :
    HTMLReport(p, f, df, dl)
{
    tab = new HTMLResourceReportElement(this, df, dl);
}

bool
HTMLResourceReport::generate()
{
    if (!open())
        return FALSE;

    generateHeader();
    tab->generate();
    generateFooter();

    f.close();
    return TRUE;
}
