/*
 * HTMLMonthlyCalendarElement.cpp - TaskJuggler
 *
 * Copyright (c) 2006 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: HTMLMonthlyCalendarElement.cpp 1313 2006-07-27 10:50:04Z cs $
 */

#include "HTMLMonthlyCalendarElement.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Report.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "CoreAttributesList.h"
#include "Task.h"
#include "TaskList.h"
#include "Resource.h"
#include "ResourceList.h"
#include "Utility.h"
#include "Interval.h"
#include "TableColumnInfo.h"
#include "TableLineInfo.h"
#include "HTMLReport.h"

HTMLMonthlyCalendarElement::HTMLMonthlyCalendarElement(Report* r,
                                                       const QString& df,
                                                       int dl) :
    HTMLReportElement(r, df, dl)
{
    uint sc = r->getProject()->getMaxScenarios();
    columns.append(new TableColumnInfo(sc, "name"));

    // show all tasks
    setHideTask(new ExpressionTree(new Operation(0)));
    // hide all resources
    setHideResource(new ExpressionTree(new Operation(1)));

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;

    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
    resourceSortCriteria[1] = CoreAttributesList::NameUp;
    resourceSortCriteria[2] = CoreAttributesList::IdUp;

}

HTMLMonthlyCalendarElement::~HTMLMonthlyCalendarElement()
{
}

void
HTMLMonthlyCalendarElement::generateTableHeader()
{
    s() << " <thead>" << endl
        << "   <tr style=\"background-color:"
        << colors.getColorName("header")
        << "; text-align:center\">" << endl;
    time_t calStart = beginOfMonth(start);
    time_t calEnd = sameTimeNextMonth(beginOfMonth(end));
    for (int month = calStart; month < calEnd; month =
         sameTimeNextMonth(month))
    {
        s() << "   <th style=\"font-size:110%;\">"
            << htmlFilter(monthAndYear(month)) << "</th>" << endl;
    }
    s() << "  </tr>" << endl
        << " </thead>" << endl;
}

void
HTMLMonthlyCalendarElement::generateTaksPerMonth(TaskList& filteredTaskList)
{
    // Generate a row with lists the tasks for each day.
    s() << "  <tr style=\"background-color:"
        << colors.getColorName("default") << "\">" << endl
        << endl;
    time_t calStart = beginOfMonth(start);
    time_t calEnd = sameTimeNextMonth(beginOfMonth(end));
    for (int month = calStart; month < calEnd; month =
         sameTimeNextMonth(month))
    {
        /* Misuse the class member start and end to limit the scope of
         * the information listed. */
        time_t savedStart = start;
        time_t savedEnd = end;
        start = month;
        end = sameTimeNextMonth(month);

        s() << "   <td style=\"vertical-align:top\">" << endl;
        bool first = TRUE;
        int no = 1;
        for (TaskListIterator tli(filteredTaskList); *tli != 0;
             ++tli, ++no)
        {
            if ((*tli)->getLoad(scenarios[0], Interval(start, end)) == 0.0)
                continue;
            if ((*tli)->isActive(scenarios[0],
                                 Interval(month, sameTimeNextMonth(month))))
            {
                if (first)
                {
                    s() << "     <table width=\"100%\">" << endl;
                    first = FALSE;
                }
                TableLineInfo tli1;
                tli1.ca1 = tli1.task = *tli;
                tli1.idxNo = no;
                tli1.fontFactor = 40;
                generateLine(&tli1, 2);
            }
        }
        if (!first)
            s() << "     </table>" << endl;
        s() << "   </td>" << endl;
        start = savedStart;
        end = savedEnd;
    }
    s() << "  </tr>" << endl;
}

bool
HTMLMonthlyCalendarElement::generate()
{
    generateHeader();

    TaskList filteredTaskList;
    if (!filterTaskList(filteredTaskList, 0, hideTask, rollUpTask))
        return FALSE;
    sortTaskList(filteredTaskList);
    maxDepthTaskList = filteredTaskList.maxDepth();

    s() << "<table align=\"center\" cellpadding=\"2\" "
        << "style=\"background-color:#000000\"";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_table\"";
    s() << ">" << endl;

    generateTableHeader();

    s()    << " <tbody>" << endl;

    if (!filteredTaskList.isEmpty())
        generateTaksPerMonth(filteredTaskList);


    s() << " </tbody>" << endl << "</table>" << endl;

    generateFooter();

    return TRUE;
}

