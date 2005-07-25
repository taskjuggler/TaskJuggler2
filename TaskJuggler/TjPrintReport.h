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

#ifndef _TjPrintReport_h_
#define _TjPrintReport_h_

#include "TjReportRow.h"

#include <qptrlist.h>

#include "TaskList.h"
#include "ResourceList.h"

class QPaintDevice;
class QPainter;
class Report;
class TaskList;

class TjPrintReport
{
public:
    TjPrintReport(Report* const rd);
    virtual ~TjPrintReport();

    virtual bool generate(QPaintDevice* pd) = 0;

    virtual void printReportPage(QPaintDevice* pd, int x, int y) = 0;

protected:
    Report* const reportDef;
    int scenario;
    TaskList taskList;
    ResourceList resourceList;
    QPainter* p;

    QPtrList<TjReportRow> rows;
} ;

#endif
