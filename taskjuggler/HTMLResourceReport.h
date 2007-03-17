/*
 * HTMLResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLResourceReport_h_
#define _HTMLResourceReport_h_

#include "HTMLReport.h"
#include "HTMLResourceReportElement.h"

/**
 * @short Stores all information about an HTML resource report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLResourceReport : public HTMLReport
{
public:
    HTMLResourceReport(Project* p, const QString& f, const QString& df, int dl) :
	    HTMLReport(p, f, df, dl ),
	    tab(this, df, dl)
    { }
    virtual ~HTMLResourceReport()
    { }

    virtual const char* getType() const { return "HTMLResourceReport"; }

    bool generate();
    HTMLResourceReportElement* getTable() { return &tab; }

private:
    HTMLResourceReportElement tab;
};

#endif
