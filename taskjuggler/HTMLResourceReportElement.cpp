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
    filterTaskList(filteredTaskList, 0, hideTask, rollUpTask);
    maxDepthTaskList = filteredTaskList.maxDepth();
    
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
            tli1.bgCol = colors.getColor("default").dark(100 + sc * 10);
            generateLine(&tli1, sc == 0 ? 4 : 5);
        }

        /* We only want to show the nested task list for leaf resources. Leaf
         * in this case means "task has no visible childs". */
        bool hasVisibleChilds = FALSE;
        for (ResourceListIterator cli((*rli)->getSubListIterator());
             *cli; ++cli)
             if (filteredResourceList.findRef(*cli) >= 0)
             {
                 hasVisibleChilds = TRUE;
                 break;
             }

        if (hasVisibleChilds)
            continue;

        filterTaskList(filteredTaskList, *rli, hideTask, rollUpTask);
        sortTaskList(filteredTaskList);

        int tNo = 1;
        for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli, ++tNo)
        {
            TableLineInfo tli2;
            tli2.ca1 = tli2.task = *tli;
            tli2.ca2 = tli2.resource = *rli;
            for (uint sc = 0; sc < scenarios.count(); ++sc)
            {
                tli2.row = sc;
                tli2.sc = scenarios[sc];
                tli2.idxNo = tNo;
                tli2.bgCol = colors.getColor("default").light(120).
                    dark(100 + sc * 10);
                generateLine(&tli2, sc == 0 ? 2 : 3);
            }
        }
    }
    s() << "</tbody>" << endl;
    s() << "</table>" << endl;
    generateFooter();
}

