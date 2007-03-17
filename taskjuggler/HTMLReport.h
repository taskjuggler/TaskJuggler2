/*
 * HTMLReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLReport_h_
#define _HTMLReport_h_

#include "Report.h"
#include "HTMLPrimitives.h"

class Project;
class ExpressionTree;

/**
 * @short Stores all information about an HTML report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLReport : public Report, public HTMLPrimitives
{
public:
    HTMLReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~HTMLReport() { }

    virtual const char* getType() const { return "HTMLReport"; }

    void generateHeader(const QString& title);
    void generateFooter();

    void setRawStyleSheet(const QString& ss)
    {
        rawStyleSheet = ss;
    }
    bool hasStyleSheet() const { return !rawStyleSheet.isEmpty(); }

    void setRawHead(const QString& head)
    {
        rawHead = head;
    }

    void setRawTail(const QString& tail)
    {
        rawTail = tail;
    }

protected:
    QString rawStyleSheet;
    QString rawHead;
    QString rawTail;
} ;

#endif
