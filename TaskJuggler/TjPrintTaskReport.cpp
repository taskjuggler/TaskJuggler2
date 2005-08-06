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
#include <qpaintdevicemetrics.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "ResourceList.h"
#include "ReportElement.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"

bool
TjPrintTaskReport::generate()
{
    if (!reportDef)
        return FALSE;

    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement =
        (dynamic_cast<const QtTaskReport*>(reportDef))->getTable();
    scenario = reportElement->getScenario(0);
    TaskList filteredTaskList = reportDef->getProject()->getTaskList();
    resourceList = reportDef->getProject()->getResourceList();

    /* Get complete task list, filter and sort it. Then determine the maximum
     * tree level. */
    if (!reportElement->filterTaskList(taskList, 0,
                                       reportElement->getHideTask(),
                                       reportElement->getRollUpTask()))
        return FALSE;
    reportElement->sortTaskList(filteredTaskList);
    maxDepthTaskList = filteredTaskList.maxDepth();

    /* Same for resource list. Just that we don't have to sort it. It needs to
     * be regenerated per task later on. */
    ResourceList filteredResourceList;
    if (!reportElement->filterResourceList(filteredResourceList, 0,
                                           reportElement->getHideResource(),
                                           reportElement->getRollUpResource()))
        return FALSE;
    maxDepthResourceList = filteredResourceList.maxDepth();

    generateTableHeader();

    for (TaskListIterator tli(taskList); *tli; ++tli)
    {
        TjReportRow* row = new TjReportRow(getNumberOfColumns());
        row->setCoreAttributes(static_cast<const CoreAttributes*>(*tli));
        rows.append(row);

        generateTaskListRow(row, *tli);

        ResourceList filteredResourceList;
        if (!reportElement->filterResourceList
            (filteredResourceList, *tli, reportElement->getHideResource(),
             reportElement->getRollUpResource()))
            return FALSE;
        reportElement->sortResourceList(filteredResourceList);
        for (ResourceListIterator rli(filteredResourceList); *rli; ++rli)
        {
            row = new TjReportRow(getNumberOfColumns());
            row->setCoreAttributes(static_cast<const CoreAttributes*>(*rli));
            rows.append(row);

            generateResourceListRow(row, *rli, *tli);
        }
    }

    computeTableMetrics();

    return TRUE;
}

