/*
 * CSVResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CSVResourceReport_h_
#define _CSVResourceReport_h_

#include "CSVReport.h"

class Project;
class CSVResourceReportElement;

/**
 * @short Stores all information about an CSV resource report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class CSVResourceReport : public CSVReport
{
public:
    CSVResourceReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~CSVResourceReport() { }

    bool generate();
    CSVResourceReportElement* getTable() { return tab; }

private:
    CSVResourceReport() { }

    CSVResourceReportElement* tab;
} ;

#endif
