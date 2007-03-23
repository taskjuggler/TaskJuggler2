/*
 * QtResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtResourceReport_h_
#define _QtResourceReport_h_

#include "QtReport.h"
#include "QtResourceReportElement.h"

/**
 * @short Stores all information about an Qt task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class QtResourceReport : public QtReport
{
public:
    QtResourceReport(Project* p, const QString& f, const QString& df, int dl) :
        QtReport(p, f, df, dl)
    { 
        setTable(new QtResourceReportElement(this, df, dl));
    }

    virtual ~QtResourceReport()
    { }

    virtual const char* getType() const { return "QtResourceReport"; }
} ;

#endif
