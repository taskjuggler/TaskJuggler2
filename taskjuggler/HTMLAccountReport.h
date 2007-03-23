/*
 * HTMLAccountReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLAccountReport_h_
#define _HTMLAccountReport_h_

#include "HTMLSingleReport.h"
#include "HTMLAccountReportElement.h"

/**
 * @short Stores all information about an HTML account report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLAccountReport : public HTMLSingleReport
{
public:
    HTMLAccountReport(Project* p, const QString& f, const QString& df, int dl) :
        HTMLSingleReport(p, f, df, dl)
    {
        setTable(new HTMLAccountReportElement(this, df, dl));
    }

    virtual ~HTMLAccountReport()
    { }

    virtual const char* getType() const { return "HTMLAccountReport"; }
    virtual QString getTitle() const { return i18n("Account Report"); }
};

#endif
