/*
 * CSVTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CSVTaskReport_h_
#define _CSVTaskReport_h_

#include "CSVReport.h"

class Project;
class CSVTaskReportElement;

/**
 * @short Stores all information about an CSV task report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class CSVTaskReport : public CSVReport
{
public:
    CSVTaskReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~CSVTaskReport();

    bool generate();
    CSVTaskReportElement* getTable() { return tab; }

private:
    CSVTaskReport() { }

    CSVTaskReportElement* tab;
} ;

#endif
