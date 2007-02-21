/*
 * CSVAccountReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CSVAccountReport_h_
#define _CSVAccountReport_h_

#include "CSVReport.h"
#include "CSVAccountReportElement.h"

/**
 * @short Stores all information about an CSV account report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class CSVAccountReport : public CSVReport
{
public:
    CSVAccountReport(Project* p, const QString& f, const QString& df, int dl) :
        CSVReport(p, f, df, dl),
        tab(this, df, dl)
    { }

    virtual ~CSVAccountReport()
    { }

    virtual const char* getType() const { return "CSVAccountReport"; }

    bool generate();
    CSVAccountReportElement* getTable() { return &tab; }

private:
    CSVAccountReport(); // leave unimplemented

    CSVAccountReportElement tab;
};

#endif
