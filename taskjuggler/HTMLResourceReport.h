/*
 * HTMLResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLResourceReport_h_
#define _HTMLResourceReport_h_

#include "HTMLReport.h"

class Project;
class HTMLResourceReportElement;

/**
 * @short Stores all information about an HTML resource report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLResourceReport : public HTMLReport
{
public:
    HTMLResourceReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLResourceReport() { }

    bool generate();
    HTMLResourceReportElement* getTable() { return tab; }

private:
    HTMLResourceReport() { }

    HTMLResourceReportElement* tab;
} ;

#endif
