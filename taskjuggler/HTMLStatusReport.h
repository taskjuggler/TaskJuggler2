/*
 * HTMLStatusReport.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLStatusReport_h_
#define _HTMLStatusReport_h_

#include "HTMLReport.h"

class Project;
class TaskList;
class HTMLReportElement;

/**
 * @short A class that generates HTML status reports. 
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLStatusReport : public HTMLReport
{
public:
    HTMLStatusReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLStatusReport();

    void setTable(int tabIdx, HTMLReportElement* tab);
    HTMLReportElement* getTable(int tabIdx) const;

    bool generate();

private:
    HTMLStatusReport() { }  // don't call this directly

    static const int tablesCount = 4;
    HTMLReportElement* tables[tablesCount];
} ;

#endif

