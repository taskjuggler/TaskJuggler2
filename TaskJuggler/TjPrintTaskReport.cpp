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

#include "TjPrintTaskReport.h"

#include <qpaintdevice.h>
#include <qpainter.h>

#include "Project.h"
#include "ReportElement.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"

bool
TjPrintTaskReport::generate(QPaintDevice* pd)
{
    if (!reportDef)
        return FALSE;

    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement =
        (dynamic_cast<QtTaskReport*>(reportDef))->getTable();
    scenario = reportElement->getScenario(0);
    taskList = reportDef->getProject()->getTaskList();

    p->begin(pd);

    p->drawRect(100, 100, 1000, 500);

    return TRUE;
}

void
TjPrintTaskReport::printReportPage(QPaintDevice* pd, int x, int y)
{
    p->setClipRect(1000 * x, 3000 * y, 1000, 3000);
}

