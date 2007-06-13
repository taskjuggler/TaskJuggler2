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

#include "TjPrintTaskReport.h"

#include <assert.h>

#include <qpaintdevice.h>
#include <qpaintdevicemetrics.h>

#include "Project.h"
#include "Resource.h"
#include "QtTaskReport.h"

void
TjPrintTaskReport::initialize()
{
    assert(reportDef != 0);

    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement = dynamic_cast<QtReportElement*>(const_cast<QtTaskReport*>(
                dynamic_cast<const QtTaskReport*>(reportDef))->getTable());
    scenario = reportElement->getScenario(0);
}

bool
TjPrintTaskReport::generate()
{
    /* Get complete task list, filter and sort it. Then determine the maximum
     * tree level. */
    TaskList filteredTaskList;
    if (!reportElement->filterTaskList(filteredTaskList, 0,
                                       reportElement->getHideTask(),
                                       reportElement->getRollUpTask()))
        return false;
    maxDepthTaskList = filteredTaskList.maxDepth();
    (static_cast<const QtTaskReportElement*>(reportElement))->
        sortTaskList(filteredTaskList);
    if (filteredTaskList.isEmpty())
        return true;

    /* Same for resource list. Just that we don't have to sort it. It needs to
     * be regenerated per task later on. */
    ResourceList filteredResourceList;
    if (!reportElement->filterResourceList(filteredResourceList, 0,
                                           reportElement->getHideResource(),
                                           reportElement->getRollUpResource()))
        return false;
    maxDepthResourceList = filteredResourceList.maxDepth();

    generateTableHeader();

    int index = 1;
    for (TaskListIterator tli(filteredTaskList); *tli; ++tli)
    {
        TjReportRow* row = new TjReportRow(getNumberOfColumns(), index++);
        row->setCoreAttributes(static_cast<CoreAttributes*>(*tli), 0);
        rows.push_back(row);

        generateTaskListRow(row, *tli);

        if (!reportElement->filterResourceList
            (filteredResourceList, *tli, reportElement->getHideResource(),
             reportElement->getRollUpResource()))
            return false;
        reportElement->sortResourceList(filteredResourceList);
        for (ResourceListIterator rli(filteredResourceList); *rli; ++rli)
        {
            row = new TjReportRow(getNumberOfColumns(), index++);
            row->setCoreAttributes(static_cast<CoreAttributes*>(*tli),
                                   static_cast<CoreAttributes*>(*rli));
            rows.push_back(row);

            generateResourceListRow(row, *rli, *tli);
        }
    }

    layoutPages();

    return true;
}

