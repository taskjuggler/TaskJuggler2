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

#include "TjPrintResourceReport.h"

#include <qpaintdevice.h>
#include <qpaintdevicemetrics.h>

#include "Project.h"
#include "Resource.h"
#include "QtResourceReport.h"

void
TjPrintResourceReport::initialize()
{
    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement = dynamic_cast<QtReportElement*>(dynamic_cast<const QtReport*>(reportDef)->getTable());
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
        return false;
    maxDepthResourceList = filteredResourceList.maxDepth();
    (static_cast<const QtResourceReportElement*>(reportElement))->
        sortResourceList(filteredResourceList);
    if (filteredResourceList.isEmpty())
        return true;

    /* Same for task list. Just that we don't have to sort it. It needs to
     * be regenerated per task later on. */
    TaskList filteredTaskList;
    if (!reportElement->filterTaskList(filteredTaskList, 0,
                                       reportElement->getHideTask(),
                                       reportElement->getRollUpTask()))
        return false;
    maxDepthTaskList = filteredTaskList.maxDepth();

    generateTableHeader();

    int index = 1;
    for (ResourceListIterator rli(filteredResourceList); *rli; ++rli)
    {
        TjReportRow* row = new TjReportRow(getNumberOfColumns(), index++);
        row->setCoreAttributes(*rli, 0);
        rows.push_back(row);

        generateResourceListRow(row, *rli);

        if (!reportElement->filterTaskList
            (filteredTaskList, *rli, reportElement->getHideTask(),
             reportElement->getRollUpTask()))
            return false;
        reportElement->sortTaskList(filteredTaskList);
        for (TaskListIterator tli(filteredTaskList); *tli; ++tli)
        {
            row = new TjReportRow(getNumberOfColumns(), index++);
            row->setCoreAttributes(*rli, *tli);
            rows.push_back(row);

            generateTaskListRow(row, *tli, *rli);
        }
    }

    layoutPages();

    return true;
}

