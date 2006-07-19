/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjPrintTaskReport_h_
#define _TjPrintTaskReport_h_

#include "TjPrintReport.h"

class KPrinter;
class QtTaskReportElement;

class TjPrintTaskReport : public TjPrintReport
{
public:
    TjPrintTaskReport(Report* const rd, KPrinter* pr) :
        TjPrintReport(rd, pr) { }
    ~TjPrintTaskReport() { }

    virtual void initialize();
    virtual bool generate();
} ;

#endif

