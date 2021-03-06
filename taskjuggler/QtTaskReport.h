/*
 * QtTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtTaskReport_h_
#define _QtTaskReport_h_

#include "QtReport.h"
#include "QtTaskReportElement.h"

/**
 * @short Stores all information about an Qt task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class QtTaskReport : public QtReport
{
public:
    QtTaskReport(Project* p, const QString& f, const QString& df, int dl) :
        QtReport(p, f, df, dl)
    { 
        setTable(new QtTaskReportElement(this, df, dl));
        taskSortCriteria[0] = CoreAttributesList::TreeMode;
        taskSortCriteria[1] = CoreAttributesList::StartUp;
        taskSortCriteria[2] = CoreAttributesList::EndUp;
        resourceSortCriteria[0] = CoreAttributesList::TreeMode;
    }

    virtual ~QtTaskReport()
    { }

    virtual const char* getType() const { return "QtTaskReport"; }
} ;

#endif
