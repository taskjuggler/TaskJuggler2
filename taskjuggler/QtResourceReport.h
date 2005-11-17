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

class Project;
class QtResourceReportElement;

/**
 * @short Stores all information about an Qt task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class QtResourceReport : public QtReport
{
public:
    QtResourceReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~QtResourceReport();

    virtual const char* getType() const { return "QtResourceReport"; }

    bool generate() { return FALSE; }

    QtResourceReportElement* getTable() const { return tab; }

private:
    QtResourceReport() { }

    QtResourceReportElement* tab;
} ;

#endif
