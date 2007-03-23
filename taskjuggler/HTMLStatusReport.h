/*
 * HTMLStatusReport.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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

#include "tjlib-internal.h"

#include <qptrvector.h>

class Project;
class TaskList;
class HTMLReportElement;

/**
 * @short A class that generates HTML status reports.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLStatusReport : public HTMLReport
{
public:
    HTMLStatusReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLStatusReport();

    virtual const char* getType() const { return "HTMLStatusReport"; }
    virtual QString getTitle() const { return i18n("Status Report"); }

    void setTable(int tabIdx, HTMLReportElement* tab);
    HTMLReportElement* getTable(int tabIdx) const;

    bool generate();

private:
    QPtrVector< HTMLReportElement > tables;
} ;

#endif

