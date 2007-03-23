/*
 * CSVResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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
#include "CSVResourceReportElement.h"

/**
 * @short Stores all information about an CSV resource report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class CSVResourceReport : public CSVReport
{
public:
    CSVResourceReport(Project* p, const QString& f, const QString& df, int dl) :
        CSVReport(p, f, df, dl)
    {
        setTable(new CSVResourceReportElement(this, df, dl));
    }

    virtual ~CSVResourceReport() { }
} ;

#endif
