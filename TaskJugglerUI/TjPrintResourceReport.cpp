/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#include "TjPrintResourceReport.h"

#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "ResourceList.h"
#include "ReportElement.h"
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"

void
TjPrintResourceReport::initialize()
{
    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement =
        (dynamic_cast<const QtResourceReport*>(reportDef))->getTable();
    scenario = reportElement->getScenario(0);
}

bool
TjPrintResourceReport::generate()
{
    /* Get complete resource list, filter and sort it. Then determine the
     * maximum tree level. */
    ResourceList filteredResourceList;
    if (!reportElement->filterResourceList(filteredResourceList, 0,
                                           reportElement->getHideResource(),
                                           reportElement->getRollUpResource()))
        return FALSE;
    maxDepthResourceList = filteredResourceList.maxDepth();
    (static_cast<const QtResourceReportElement*>(reportElement))->
        sortResourceList(filteredResourceList);
    if (filteredResourceList.isEmpty())
        return TRUE;

    /* Same for task list. Just that we don't have to sort it. It needs to
     * be regenerated per task later on. */
    TaskList filteredTaskList;
    if (!reportElement->filterTaskList(filteredTaskList, 0,
                                       reportElement->getHideTask(),
                                       reportElement->getRollUpTask()))
        return FALSE;
    maxDepthTaskList = filteredTaskList.maxDepth();

    generateTableHeader();

    int index = 1;
    for (ResourceListIterator rli(filteredResourceList); *rli; ++rli)
    {
        TjReportRow* row = new TjReportRow(getNumberOfColumns(), index++);
        row->setCoreAttributes(static_cast<const CoreAttributes*>(*rli), 0);
        rows.push_back(row);

        generateResourceListRow(row, *rli);

        if (!reportElement->filterTaskList
            (filteredTaskList, *rli, reportElement->getHideTask(),
             reportElement->getRollUpTask()))
            return FALSE;
        reportElement->sortTaskList(filteredTaskList);
        for (TaskListIterator tli(filteredTaskList); *tli; ++tli)
        {
            row = new TjReportRow(getNumberOfColumns(), index++);
            row->setCoreAttributes(static_cast<const CoreAttributes*>(*rli),
                                   static_cast<const CoreAttributes*>(*tli));
            rows.push_back(row);

            generateTaskListRow(row, *tli, *rli);
        }
    }

    layoutPages();

    return TRUE;
}

