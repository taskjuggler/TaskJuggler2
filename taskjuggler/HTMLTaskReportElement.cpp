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
#include "TableColumn.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Report.h"
#include "TaskList.h"
#include "ResourceList.h"
#include "CoreAttributes.h"

HTMLTaskReportElement::HTMLTaskReportElement(Report* r, const QString& df,
                                             int dl) :
    HTMLReportElement(r, df, dl)
{
    columns.append(new TableColumn("no"));
    columns.append(new TableColumn("name"));
    columns.append(new TableColumn("start"));
    columns.append(new TableColumn("end"));

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
        generateFirstTask(*tli, 0, tNo);
        for (uint sc = 1; sc < scenarios.count(); ++sc)
            generateNextTask(scenarios[sc], *tli, 0);

        filterResourceList(filteredResourceList, *tli, 
                           getHideResource(), getRollUpResource());
        sortResourceList(filteredResourceList);
        int rNo = 1;
        for (ResourceListIterator rli(filteredResourceList); *rli != 0; 
             ++rli, ++rNo)
        {
            generateFirstResource(*rli, *tli, rNo);
            for (uint sc = 1; sc < scenarios.count(); ++sc)
                generateNextResource(scenarios[sc], *rli, *tli);
        }
    }
    s() << " </tbody>" << endl;
    s() << "</table>" << endl;

    generateFooter();
}

