/*
 * CSVResourceReportElement.h - ResourceJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CSVResourceReportElement.h"
#include "TableColumnInfo.h"
#include "TableLineInfo.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Report.h"
#include "Project.h"
#include "Task.h"
#include "TaskList.h"
#include "Resource.h"
#include "ResourceList.h"
#include "CoreAttributes.h"

CSVResourceReportElement::CSVResourceReportElement(Report* r,
                                                     const QString& df,
                                                     int dl) :
    CSVReportElement(r, df, dl)
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

CSVResourceReportElement::~CSVResourceReportElement()
{
}

bool
CSVResourceReportElement::generate()
{
    generateHeader();

    generateTableHeader();

    ResourceList filteredResourceList;
    if (!filterResourceList(filteredResourceList, 0, hideResource,
                            rollUpResource))
        return FALSE;
    sortResourceList(filteredResourceList);
    maxDepthResourceList = filteredResourceList.maxDepth();

    maxDepthTaskList = 0;
    
    int rNo = 1;
    for (ResourceListIterator rli(filteredResourceList); *rli != 0; 
         ++rli, ++rNo)
    {
        TableLineInfo tli1;
        tli1.ca1 = tli1.resource = *rli;
        for (uint sc = 0; sc < scenarios.count(); ++sc)
        {
            tli1.row = sc;
            tli1.sc = scenarios[sc];
            tli1.idxNo = rNo;
            generateLine(&tli1, sc == 0 ? 4 : 5);
        }
    }
    generateFooter();

    return TRUE;
}

