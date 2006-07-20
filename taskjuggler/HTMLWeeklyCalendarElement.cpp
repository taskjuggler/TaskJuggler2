/*
 * HTMLWeeklyCalendarElement.cpp - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLWeeklyCalendarElement.h"
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

HTMLWeeklyCalendarElement::HTMLWeeklyCalendarElement(Report* r,
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

HTMLWeeklyCalendarElement::~HTMLWeeklyCalendarElement()
{
}

bool
HTMLWeeklyCalendarElement::generate()
{
    generateHeader();

    TaskList filteredTaskList;
    if (!filterTaskList(filteredTaskList, 0, hideTask, rollUpTask))
        return FALSE;
    sortTaskList(filteredTaskList);
    maxDepthTaskList = filteredTaskList.maxDepth();

    ResourceList filteredResourceList;
    if (!filterResourceList(filteredResourceList, 0, hideResource,
                            rollUpResource))
        return FALSE;
    sortResourceList(filteredResourceList);
    maxDepthResourceList = filteredResourceList.maxDepth();

    bool weekStartsMonday = report->getProject()->getWeekStartsMonday();
    s() << "<table align=\"center\" cellpadding=\"2\" "
        << "style=\"background-color:#000000\"";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_table\"";
    s() << ">" << endl;

    s() << " <thead>" << endl
        << "   <tr style=\"background-color:"
        << colors.getColorName("header")
        << "; text-align:center\">" << endl;
    time_t wd = beginOfWeek(start, weekStartsMonday);
    for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
    {
        s() << "   <th width=\"14.2%\" style=\"font-size:110%; ";
        if (isWeekend(wd))
            s() << "background-color:"
                << colors.getColor("header").dark(130).name();
        s() << "\">"
            << htmlFilter(dayOfWeekName(wd)) << "</th>" << endl;
    }
    s() << "  </tr>" << endl
        << " </thead>" << endl
        << " <tbody>" << endl;

    QString lastMAY;
    for (time_t week = beginOfWeek(start, weekStartsMonday);
         week <= sameTimeNextWeek(beginOfWeek(end, weekStartsMonday)) - 1; )
    {
        time_t wd = week;
        /* Generate table row that contains the day of the month, the month
         * and the year. The first column of the row also has the number of
         * the week. */
        s() << "  <tr style=\"background-color:"
            << colors.getColorName("header")
            << "; text-align:center\">" << endl;
        for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
        {
            s() << "   <td width=\"14.2%\"";
            if (isSameDay(report->getProject()->getNow(), wd))
               s() << " style=\"background-color:"
                  << colors.getColorName("today") << "\"";
            else if (isWeekend(wd))
               s() << " style=\"background-color:"
                  << colors.getColor("header").dark(130).name() << "\"";
            s() << ">" << endl
                << "   <table width=\"100%\">"
                << endl
                << "    <tr>" << endl
                << "     <td width=\"30%\" rowspan=\"2\" "
                   "style=\"font-size:200%; text-align:center\">"
                << QString().sprintf("%d", dayOfMonth(wd)) << "</td>" << endl
                << "     <td width=\"70%\" style=\"font-size:60%\">";
            if (day == 0)
                s() << htmlFilter(i18n("Week")) << " "
                   << QString("%1").arg(weekOfYear(wd, weekStartsMonday));
            else
                s() << "<p></p>";
            s() << "     </td>" << endl
                << "    </tr>" << endl
                << "    <tr>" << endl;
            QString mAY = monthAndYear(wd);
            if (mAY != lastMAY)
            {
                s() << "     <td style=\"font-size:90%\">"
                    << monthAndYear(wd) << "</td>" << endl;
                lastMAY = mAY;
            }
            s() << "    </tr>" << endl;
            if (report->getProject()->isVacation(wd))
                s() << "    <tr><td colspan=\"2\" style=\"font-size:80%\">"
                    << report->getProject()->vacationName(wd)
                    << "</td></tr>" << endl;
            s() << "   </table></td>" << endl;
        }
        s() << "  </tr>" << endl;

        if (!filteredTaskList.isEmpty())
        {
            // Generate a row with lists the tasks for each day.
            s() << "  <tr style=\"background-color:"
                << colors.getColorName("default") << "\">" << endl
                << endl;
            wd = week;
            for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
            {
                /* Misuse the class member start and end to limit the scope of
                 * the information listed. */
                time_t savedStart = start;
                time_t savedEnd = end;
                start = wd;
                end = sameTimeNextDay(wd);
                s() << "   <td width=\"14.2%\" style=\"vertical-align:top\">"
                    << endl;
                bool first = TRUE;
                int no = 1;
                for (TaskListIterator tli(filteredTaskList); *tli != 0;
                     ++tli, ++no)
                {
                    if ((*tli)->isActive(scenarios[0],
                                         Interval(wd, sameTimeNextDay(wd))))
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

        if (!filteredResourceList.isEmpty())
        {
            // Generate a table row which lists the resources for each day.
            s() << "  <tr style=\"background-color:"
                << colors.getColorName("default") << "\">" << endl
                << endl;
            wd = week;
            for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
            {
                /* Misuse the class member start and end to limit the scope of
                 * the information listed. */
                time_t savedStart = start;
                time_t savedEnd = end;
                start = wd;
                end = sameTimeNextDay(wd);
                s() << "   <td width=\"13.5%\" style=\"vertical-align:top\">"
                    << endl;
                bool first = TRUE;
                int no = 1;
                for (ResourceListIterator rli(filteredResourceList);
                     *rli != 0; ++rli, ++no)
                {
                    if ((*rli)->getLoad(scenarios[0],
                                        Interval(wd,
                                                 sameTimeNextDay(wd))) > 0.0)
                    {
                        if (first)
                        {
                            s() << "     <table width=\"100%\">" << endl;
                            first = FALSE;
                        }
                        TableLineInfo tli2;
                        tli2.ca1 = tli2.resource = *rli;
                        tli2.idxNo = no;
                        tli2.fontFactor = 40;
                        generateLine(&tli2, 4);
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

        week = wd;
    }

    s() << " </tbody>" << endl << "</table>" << endl;

    generateFooter();

    return TRUE;
}

