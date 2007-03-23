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

#include "HTMLSingleReport.h"
#include "HTMLTaskReportElement.h"

/**
 * @short Stores all information about an HTML task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLTaskReport : public HTMLSingleReport
{
public:
    HTMLTaskReport(Project* p, const QString& f, const QString& df, int dl) :
	    HTMLSingleReport(p, f, df, dl)
    {
	    setTable(new HTMLTaskReportElement(this, df, dl));
    }

    virtual ~HTMLTaskReport()
    { }

    virtual const char* getType() const { return "HTMLTaskReport"; }
    virtual QString getTitle() const { return i18n("Task Report"); }
};

#endif
