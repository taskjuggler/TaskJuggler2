/*
 * QtReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtReport_h_
#define _QtReport_h_

#include "Report.h"

class Project;
class ExpressionTree;

/**
 * @short Stores all information about a Qt report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class QtReport : public Report
{
public:
    QtReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~QtReport() { }

    virtual const char* getType() const { return "QtReport"; }

    void generateHeader();
    void generateFooter();

protected:
    QtReport() { }
} ;

#endif
