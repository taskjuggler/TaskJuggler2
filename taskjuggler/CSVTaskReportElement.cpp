/*
 * CSVTaskReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CSVTaskReportElement.h"
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

CSVTaskReportElement::CSVTaskReportElement(Report* r, const QString& df,
                                             int dl) :
    CSVReportElement(r, df, dl)
{
    int sc = r->getProject()->getMaxScenarios();
    columns.append(new TableColumnInfo(sc, "no"));
    columns.append(new TableColumnInfo(sc, "name"));
    columns.append(new TableColumnInfo(sc, "start"));
    columns.append(new TableColumnInfo(sc, "end"));

    // show all tasks
    setHideTask(new ExpressionTree(new Operation(0)));
    // hide all resources
    setHideResource(new ExpressionTree(new Operation(1)));

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;
    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
}

CSVTaskReportElement::~CSVTaskReportElement()
{
}

bool
CSVTaskReportElement::generate()
{
    generateHeader();
    
    generateTableHeader();

    TaskList filteredTaskList;
    if (!filterTaskList(filteredTaskList, 0, getHideTask(), getRollUpTask()))
        return FALSE;
    sortTaskList(filteredTaskList);
    maxDepthTaskList = filteredTaskList.maxDepth();

    maxDepthResourceList = 0;
    
    int tNo = 1;
    for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli, ++tNo)
    {
        TableLineInfo tli1;
        tli1.ca1 = *tli;
        tli1.task = *tli;
        for (uint sc = 0; sc < scenarios.count(); ++sc)
        {
            tli1.row = sc;
            tli1.sc = scenarios[sc];
            tli1.idxNo = tNo;
            generateLine(&tli1, sc == 0 ? 2 : 3);
        }
    }

    generateFooter();

    return TRUE;
}

