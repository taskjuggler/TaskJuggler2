/*
 * HTMLReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLReport_h_
#define _HTMLReport_h_

#include <stdio.h>
#include <time.h>

#include <qstring.h>

#include "Report.h"
#include "HTMLPrimitives.h"

class Project;
class ExpressionTree;

/**
 * @short Stores all information about an HTML report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLReport : public Report, public HTMLPrimitives
{
public:
    HTMLReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLReport() { }

    void generateHeader();
    void generateFooter();

    void setRawStyleSheet(const QString& styleSheet)
    {
        rawStyleSheet = styleSheet;
    }
    void setRawHead(const QString& head)
    {
        rawHead = head;
    }

    void setRawTail(const QString& tail)
    {
        rawTail = tail;
    }

protected:
    HTMLReport() { }

    uint colDefault;
    uint colDefaultLight;
    uint colWeekend;
    uint colVacation;
    uint colAvailable;
    uint colBooked;
    uint colBookedLight;
    uint colHeader;
    uint colMilestone;
    uint colCompleted;
    uint colCompletedLight;
    uint colToday;

    QString rawHead;
    QString rawTail;
    QString rawStyleSheet;
} ;

#endif
