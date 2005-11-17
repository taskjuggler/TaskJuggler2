/*
 * HTMLTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLTaskReport_h_
#define _HTMLTaskReport_h_

#include "HTMLReport.h"

class Project;
class HTMLTaskReportElement;

/**
 * @short Stores all information about an HTML task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLTaskReport : public HTMLReport
{
public:
    HTMLTaskReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLTaskReport();

    virtual const char* getType() const { return "HTMLTaskReport"; }

    bool generate();
    HTMLTaskReportElement* getTable() { return tab; }

private:
    HTMLTaskReport() { }

    HTMLTaskReportElement* tab;
} ;

#endif
