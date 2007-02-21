/*
 * CSVTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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
#include "CSVTaskReportElement.h"

/**
 * @short Stores all information about an CSV task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class CSVTaskReport : public CSVReport
{
public:
    CSVTaskReport(Project* p, const QString& f, const QString& df, int dl) :
        CSVReport(p, f, df, dl),
        tab(this, df, dl)
    { }

    virtual ~CSVTaskReport()
    { }

    virtual const char* getType() const { return "CSVTaskReport"; }

    bool generate();
    CSVTaskReportElement* getTable() { return &tab; }

private:
    CSVTaskReport(); // leave unimplemented

    CSVTaskReportElement tab;
};

#endif
