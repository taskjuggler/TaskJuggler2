/*
 * HTMLResourceReportElement.h - ResourceJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLResourceReportElement.h"
#include "TableColumnInfo.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Report.h"
#include "Project.h"
#include "TaskList.h"
#include "ResourceList.h"
#include "CoreAttributes.h"

HTMLResourceReportElement::HTMLResourceReportElement(Report* r,
                                                     const QString& df,
                                                     int dl) :
    HTMLReportElement(r, df, dl)
{
    uint sc = r->getProject()->getMaxScenarios();
    columns.append(new TableColumnInfo(sc, "no"));
    columns.append(new TableColumnInfo(sc, "name"));
    columns.append(new TableColumnInfo(sc, "start"));
    columns.append(new TableColumnInfo(sc, "end"));

    // show all resources
    setHideResource(new ExpressionTree(new Operation(0)));
    // hide all tasks
    setHideTask(new ExpressionTree(new Operation(1)));

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;
    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
}

HTMLResourceReportElement::~HTMLResourceReportElement()
{
}

void
HTMLResourceReportElement::generate()
{
    generateHeader();

    generateTableHeader();

    s() << "<tbody>" << endl;
    
    ResourceList filteredResourceList;
    filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
    sortResourceList(filteredResourceList);
    maxDepthResourceList = filteredResourceList.maxDepth();

    TaskList filteredTaskList;
    filterTaskList(filteredTaskList, 0, hideTask, rollUpResource);
    maxDepthTaskList = filteredTaskList.maxDepth();
    
    int rNo = 1;
    for (ResourceListIterator rli(filteredResourceList); *rli != 0; 
         ++rli, ++rNo)
    {
        generateFirstResource(scenarios[0], *rli, 0, rNo);
        for (uint sc = 1; sc < scenarios.count(); ++sc)
            generateNextResource(scenarios[sc], *rli, 0);

        filterTaskList(filteredTaskList, *rli, hideTask, rollUpResource);
        sortTaskList(filteredTaskList);

        int tNo = 1;
        for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli, ++tNo)
        {
            generateFirstTask(scenarios[0], *tli, *rli, tNo);
            for (uint sc = 1; sc < scenarios.count(); ++sc)
                generateNextTask(scenarios[sc], *tli, *rli);
        }
    }
    s() << "</tbody>" << endl;
    s() << "</table>" << endl;
    generateFooter();
}

