/*
 * HTMLResourceReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLResourceReport_h_
#define _HTMLResourceReport_h_

#include <Report.h>

/**
 * @short Stores all information about an HTML resource report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLResourceReport : public ReportHtml
{
public:
    HTMLResourceReport(Project* p, const QString& f, time_t s, time_t e,
                       const QString& df, int dl);
    virtual ~HTMLResourceReport() { }

    bool generate();

private:
    HTMLResourceReport() { }
} ;

#endif
