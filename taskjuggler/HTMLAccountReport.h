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

#ifndef _HTMLAccountReport_h_
#define _HTMLAccountReport_h_

#include "HTMLReport.h"

class Project;
class HTMLAccountReportElement;

/**
 * @short Stores all information about an HTML account report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLAccountReport : public HTMLReport
{
public:
    HTMLAccountReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLAccountReport();

    virtual const char* getType() const { return "HTMLAccountReport"; }

    bool generate();
    HTMLAccountReportElement* getTable() { return tab; }

private:
    HTMLAccountReport() { } // Don't use this.

    HTMLAccountReportElement* tab;
} ;

#endif
