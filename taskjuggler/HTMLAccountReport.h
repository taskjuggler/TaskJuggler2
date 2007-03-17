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

#include "HTMLReport.h"
#include "HTMLAccountReportElement.h"

/**
 * @short Stores all information about an HTML account report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLAccountReport : public HTMLReport
{
public:
    HTMLAccountReport(Project* p, const QString& f, const QString& df, int dl) :
        HTMLReport(p, f, df, dl),
        tab(this, df, dl)
    { }

    virtual ~HTMLAccountReport()
    { }


    virtual const char* getType() const { return "HTMLAccountReport"; }

    bool generate();
    HTMLAccountReportElement* getTable() { return &tab; }

private:
    HTMLAccountReportElement tab;
};

#endif
