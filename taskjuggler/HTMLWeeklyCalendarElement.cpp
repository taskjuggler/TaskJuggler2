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
    HTMLReportElement(r, df, dl),
    daysToShow(),
    numberOfDays(7),
    taskReport(true)
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

    daysToShow.resize(7);
    for (unsigned int i = 0; i < 7; ++i)
        daysToShow.setBit(i);
}

HTMLWeeklyCalendarElement::~HTMLWeeklyCalendarElement()
{
}

void
HTMLWeeklyCalendarElement::setDaysToShow(QBitArray& days)
{
    daysToShow = days;
    numberOfDays = 0;
    for (int i = days.size() - 1; i >= 0; i--)
        if (days[i])
            numberOfDays++;
}

void
HTMLWeeklyCalendarElement::generateTableHeader(bool weekStartsMonday)
{
    s() << " <thead>" << endl
        << "   <tr style=\"background-color:"
        << colors.getColorName("header")
        << "; text-align:center\">" << endl;
    time_t wd = beginOfWeek(start, weekStartsMonday);
    QString cellWidth;
    cellWidth.sprintf("%.1f", 100.0 / numberOfDays);
    for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
    {
        if (!showThisDay(day, weekStartsMonday))
            continue;

        s() << "   <th width=\"" << cellWidth << "%\" style=\"font-size:110%; ";
        if (isWeekend(wd))
            s() << "background-color:"
                << colors.getColor("header").dark(130).name();
        s() << "\">"
            << htmlFilter(dayOfWeekName(wd)) << "</th>" << endl;
    }
    s() << "  </tr>" << endl
        << " </thead>" << endl;
}

void
HTMLWeeklyCalendarElement::generateWeekHeader(bool weekStartsMonday,
                                              time_t week)
{
    /* Generate table row that contains the day of the month, the month
     * and the year. The first column of the row also has the number of
     * the week. */
    time_t wd = week;
    s() << "  <tr style=\"background-color:"
        << colors.getColorName("header")
        << "; text-align:center\">" << endl;
    QString lastMAY;
    QString cellWidth;
    cellWidth.sprintf("%.1f", 100.0 / numberOfDays);
    for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
    {
        if (!showThisDay(day, weekStartsMonday))
            continue;

        s() << "   <td width=\"" << cellWidth << "%\"";
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
                << htmlFilter(mAY) << "</td>" << endl;
            lastMAY = mAY;
        }
        s() << "    </tr>" << endl;
        if (report->getProject()->isVacation(wd))
            s() << "    <tr><td colspan=\"2\" style=\"font-size:80%\">"
                << htmlFilter(report->getProject()->vacationName(wd))
                << "</td></tr>" << endl;
        s() << "   </table></td>" << endl;
    }
    s() << "  </tr>" << endl;
}

bool
HTMLWeeklyCalendarElement::generateTaksPerDay(
   time_t& wd, TaskList& filteredTaskList, ResourceList& filteredResourceList,
   bool weekStartsMonday)
{
    // Generate a row with lists the tasks for each day.
    s() << "  <tr style=\"background-color:"
        << colors.getColorName("default") << "\">" << endl
        << endl;
    QString cellWidth;
    cellWidth.sprintf("%.1f", 100.0 / numberOfDays);
    for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
    {
        if (!showThisDay(day, weekStartsMonday))
            continue;

        /* Misuse the class member start and end to limit the scope of
         * the information listed. */
        time_t savedStart = start;
        time_t savedEnd = end;
        start = wd;
        end = sameTimeNextDay(wd);

        s() << "   <td width=\"" << cellWidth
            << "\" style=\"vertical-align:top\">" << endl;
        bool first = TRUE;
        int no = 1;
        for (TaskListIterator tli(filteredTaskList); *tli != 0;
             ++tli, ++no)
        {
            if ((*tli)->getLoad(scenarios[0], Interval(start, end)) == 0.0)
                continue;
            if (!(*tli)->isActive(scenarios[0],
                                  Interval(wd, sameTimeNextDay(wd))))
                continue;

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

            if (!filterResourceList(filteredResourceList, *tli,
                                    getHideResource(), getRollUpResource()))
                return FALSE;
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
                    tli2.fontFactor = 30;
                    generateLine(&tli2, sc == 0 ? 4 : 5);
                }
            }
        }
        if (!first)
            s() << "     </table>" << endl;
        s() << "   </td>" << endl;
        start = savedStart;
        end = savedEnd;
    }
    s() << "  </tr>" << endl;

    return true;
}

bool
HTMLWeeklyCalendarElement::generateResourcesPerDay
    (time_t& wd, ResourceList& filteredResourceList,
     TaskList& filteredTaskList, bool weekStartsMonday)
{
    // Generate a table row which lists the resources for each day.
    s() << "  <tr style=\"background-color:"
        << colors.getColorName("default") << "\">" << endl
        << endl;
    QString cellWidth;
    cellWidth.sprintf("%.1f", 100.0 / numberOfDays);
    for (int day = 0; day < 7; ++day, wd = sameTimeNextDay(wd))
    {
        if (!showThisDay(day, weekStartsMonday))
            continue;

        /* Misuse the class member start and end to limit the scope of
         * the information listed. */
        time_t savedStart = start;
        time_t savedEnd = end;
        start = wd;
        end = sameTimeNextDay(wd);
        s() << "   <td width=\"" << cellWidth
            << "\" style=\"vertical-align:top\">"
            << endl;
        bool first = TRUE;
        int no = 1;
        for (ResourceListIterator rli(filteredResourceList);
             *rli != 0; ++rli, ++no)
        {
            if ((*rli)->getLoad(scenarios[0],
                                Interval(wd,
                                         sameTimeNextDay(wd))) <= 0.0)
                continue;

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

            /* We only want to show the nested task list for leaf resources.
             * Leaf in this case means "task has no visible childs". */
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

            if (!filterTaskList(filteredTaskList, *rli, hideTask, rollUpTask))
                return FALSE;
            sortTaskList(filteredTaskList);

            int tNo = 1;
            for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli,
                 ++tNo)
            {
                TableLineInfo tli2;
                tli2.ca1 = tli2.task = *tli;
                tli2.ca2 = tli2.resource = *rli;
                for (uint sc = 0; sc < scenarios.count(); ++sc)
                {
                    tli2.row = sc;
                    tli2.sc = scenarios[sc];
                    tli2.idxNo = tNo;
                    tli2.fontFactor = 30;
                    generateLine(&tli2, sc == 0 ? 2 : 3);
                }
            }
        }
        if (!first)
            s() << "     </table>" << endl;
        s() << "   </td>" << endl;
        start = savedStart;
        end = savedEnd;
    }
    s() << "  </tr>" << endl;

    return true;
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

    generateTableHeader(weekStartsMonday);

    s()    << " <tbody>" << endl;

    for (time_t week = beginOfWeek(start, weekStartsMonday);
         week <= sameTimeNextWeek(beginOfWeek(end, weekStartsMonday)) - 1; )
    {
        generateWeekHeader(weekStartsMonday, week);

        if (taskReport)
        {
            if (!filteredTaskList.isEmpty())
            {
                if (!generateTaksPerDay(week, filteredTaskList,
                                        filteredResourceList,
                                        weekStartsMonday))
                    return false;
            }
            else
                week = sameTimeNextWeek(week);
        }
        else
        {
            if (!filteredResourceList.isEmpty())
            {
                if (!generateResourcesPerDay(week, filteredResourceList,
                                             filteredTaskList,
                                             weekStartsMonday))
                    return false;
            }
            else
                week = sameTimeNextWeek(week);
        }
    }

    s() << " </tbody>" << endl << "</table>" << endl;

    generateFooter();

    return TRUE;
}

bool
HTMLWeeklyCalendarElement::showThisDay(int dayIndex, bool weekStartsMonday)
{
    /* The dayIndex may start with Sunday or Monday depending of the setting
     * of weekStartsMonday. daysToShow[0] is always Sunday. */

    return daysToShow[(dayIndex + (weekStartsMonday ? 1 : 0)) % 7];
}

