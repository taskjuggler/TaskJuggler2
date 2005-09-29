/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#ifndef _TjPrintTaskReport_h_
#define _TjPrintTaskReport_h_

#include "TjPrintReport.h"

class QPaintDevice;
class QtTaskReportElement;

class TjPrintTaskReport : public TjPrintReport
{
public:
    TjPrintTaskReport(Report* const rd, QPaintDevice* pd) :
        TjPrintReport(rd, pd) { }
    ~TjPrintTaskReport() { }

    virtual void initialize();
    virtual bool generate(KPrinter::Orientation orientation);
} ;

#endif

