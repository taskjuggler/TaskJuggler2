/*
 * HTMLTaskReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLTaskReportElement.h"
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

HTMLTaskReportElement::HTMLTaskReportElement(Report* r, const QString& df,
                                             int dl) :
    HTMLReportElement(r, df, dl)
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

HTMLTaskReportElement::~HTMLTaskReportElement()
{
}

void
HTMLTaskReportElement::generate()
{
    generateHeader();
    
    generateTableHeader();

    s() << " <tbody>" << endl;

    TaskList filteredTaskList;
    filterTaskList(filteredTaskList, 0, getHideTask(), getRollUpTask());
    sortTaskList(filteredTaskList);
    maxDepthTaskList = filteredTaskList.maxDepth();

    ResourceList filteredResourceList;
    filterResourceList(filteredResourceList, 0, getHideResource(),
                       getRollUpResource());
    maxDepthResourceList = filteredResourceList.maxDepth();
    
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
            tli1.bgCol = colors.getColor("default").dark(100 + sc * 10);
            generateLine(&tli1, sc == 0 ? 2 : 3);
        }

        filterResourceList(filteredResourceList, *tli, 
                           getHideResource(), getRollUpResource());
        sortResourceList(filteredResourceList);
        int rNo = 1;
        for (ResourceListIterator rli(filteredResourceList); *rli != 0; 
             ++rli, ++rNo)
        {
            TableLineInfo tli2;
            tli2.ca1 = tli2.resource = *rli;
            tli2.ca2 = tli2.task = *tli;
            for (uint sc = 0; sc < scenarios.count(); ++sc)
            {
                tli2.row = sc;
                tli2.sc = scenarios[sc];
                tli2.idxNo = rNo;
                tli2.bgCol = colors.getColor("default").light(120).
                    dark(100 + sc * 10);
                generateLine(&tli2, sc == 0 ? 4 : 5);
            }
        }
    }
    s() << " </tbody>" << endl;
    s() << "</table>" << endl;

    generateFooter();
}

