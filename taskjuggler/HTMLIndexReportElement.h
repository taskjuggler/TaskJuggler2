/*
 * HTMLIndexReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _HTMLIndexReportElement_h_
#define _HTMLIndexReportElement_h_

#include "HTMLReportElement.h"

class HTMLIndexReportElement : public HTMLReportElement
{
public:
    HTMLIndexReportElement(Report* r, const QString& df, int dl);
    ~HTMLIndexReportElement() { }

    bool generate();

private:
    bool generateReportLine(Report* report, int level = 0);
} ;

#endif

