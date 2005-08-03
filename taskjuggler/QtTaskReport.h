/*
 * QtTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
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

class Project;
class QtTaskReportElement;

/**
 * @short Stores all information about an Qt task report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class QtTaskReport : public QtReport
{
public:
    QtTaskReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~QtTaskReport();

    virtual const char* getType() const { return "QtTaskReport"; }

    bool generate() { return FALSE; }

    QtTaskReportElement* getTable() const { return tab; }

private:
    QtTaskReport() { }

    QtTaskReportElement* tab;
} ;

#endif
