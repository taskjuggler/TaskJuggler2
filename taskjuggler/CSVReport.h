/*
 * CSVReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CSVReport_h_
#define _CSVReport_h_

#include "CSVPrimitives.h"
#include "Report.h"

class Project;
class ExpressionTree;

/**
 * @short Stores all information about an CSV report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class CSVReport : public Report, public CSVPrimitives
{
public:
    CSVReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~CSVReport() { }

    virtual const char* getType() const { return "CSVReport"; }

    void generateHeader();
    void generateFooter();

protected:
    CSVReport(); // leave unimplemented
} ;

#endif
