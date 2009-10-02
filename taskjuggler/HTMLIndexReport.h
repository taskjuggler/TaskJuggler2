/*
 * HTMLIndexReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLIndexReport_h_
#define _HTMLIndexReport_h_

#include "HTMLSingleReport.h"
#include "HTMLIndexReportElement.h"
#include "tjlib-internal.h"

/**
 * @short Stores all information about an HTML report index.
*/
class HTMLIndexReport : public HTMLSingleReport
{
public:
    HTMLIndexReport(Project* p, const QString& f, const QString& df, int dl) :
        HTMLSingleReport(p, f, df, dl)
    {
        setTable(new HTMLIndexReportElement(this, df, dl));
    }

    virtual ~HTMLIndexReport()
    { }

    virtual const char* getType() const { return "HTMLIndexReport"; }
    virtual QString getTitle() const { return i18n("Report Index"); }
};

#endif
