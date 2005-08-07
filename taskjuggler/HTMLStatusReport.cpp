/*
 * HTMLStatusReport.cpp - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLStatusReport.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Project.h"
#include "Resource.h"
#include "Utility.h"
#include "HTMLReportElement.h"
#include "HTMLTaskReportElement.h"
#include "TableColumnInfo.h"
#include "tjlib-internal.h"

HTMLStatusReport::HTMLStatusReport(Project* p, const QString& f,
                                   const QString& df, int dl) :
    HTMLReport(p, f, df, dl)
{
    // By default this is a weekly status report.
    end = project->getNow();
    start = sameTimeLastWeek(end);
    if (start < project->getStart())
        start = project->getStart();
    timeFormat = "%Y-%m-%d";

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndDown;
    setHideResource(new ExpressionTree(new Operation(1)));

    for (int i = 0; i < tablesCount; ++i)
        tables[i] = new HTMLTaskReportElement(this, df, dl);

    QString scenarioName = project->getScenarioId(0);
    /* Create table that contains the tasks that should be finished, but
     * aren't. */
    tables[0]->setStart(project->getStart());
    tables[0]->setEnd(project->getEnd());
    ExpressionTree* et = new ExpressionTree();
    et->setTree(QString("~(istaskstatus(") + scenarioName + ", late))",
                project);
    tables[0]->setHideTask(et);
    tables[0]->setHeadline
        (i18n("Tasks that should have been finished already"));
    tables[0]->clearColumns();
    uint sc = p->getMaxScenarios();
    tables[0]->addColumn(new TableColumnInfo(sc, "name"));
    tables[0]->addColumn(new TableColumnInfo(sc, "duration"));
    tables[0]->addColumn(new TableColumnInfo(sc, "end"));
    tables[0]->addColumn(new TableColumnInfo(sc, "completed"));
    tables[0]->addColumn(new TableColumnInfo(sc, "resources"));
    tables[0]->addColumn(new TableColumnInfo(sc, "follows"));
    tables[0]->addColumn(new TableColumnInfo(sc, "statusnote"));

    // Ongoing tasks
    tables[1]->setStart(project->getStart());
    tables[1]->setEnd(project->getEnd());
    et = new ExpressionTree();
    et->setTree(QString("~((istaskstatus(") + scenarioName +
                ", inprogresslate) | "
                "(istaskstatus(" + scenarioName + ", inprogress))) &"
                "endsafter(" + scenarioName + "," +
                time2tjp(project->getNow()) + "))", project);
    tables[1]->setHideTask(et);
    tables[1]->setHeadline(i18n("Work in progress"));
    tables[1]->clearColumns();
    tables[1]->addColumn(new TableColumnInfo(sc, "name"));
    tables[1]->addColumn(new TableColumnInfo(sc, "duration"));
    tables[1]->addColumn(new TableColumnInfo(sc, "end"));
    tables[1]->addColumn(new TableColumnInfo(sc, "completed"));
    tables[1]->addColumn(new TableColumnInfo(sc, "resources"));
    tables[1]->addColumn(new TableColumnInfo(sc, "status"));
    tables[1]->addColumn(new TableColumnInfo(sc, "statusnote"));

    // Completed tasks
    et = new ExpressionTree();
    et->setTree(QString("~istaskstatus(") + scenarioName + ", finished)",
                project);
    tables[2]->setHideTask(et);
    tables[2]->setHeadline(i18n("Tasks that have been completed"));
    tables[2]->clearColumns();
    tables[2]->addColumn(new TableColumnInfo(sc, "name"));
    tables[2]->addColumn(new TableColumnInfo(sc, "start"));
    tables[2]->addColumn(new TableColumnInfo(sc, "end"));
    tables[2]->addColumn(new TableColumnInfo(sc, "note"));

    // Upcoming tasks
    time_t reportEnd = end + (end - start);
    if (reportEnd > project->getEnd())
        reportEnd = project->getEnd();
    tables[3]->setStart(project->getNow());
    tables[3]->setEnd(project->getEnd());
    et = new ExpressionTree();
    et->setTree(QString("~(startsafter(") + scenarioName + ", " +
                time2tjp(project->getNow()) + ") & startsbefore(" +
                scenarioName + "," + time2tjp(reportEnd) + "))", project);
    tables[3]->setHideTask(et);
    tables[3]->setHeadline(i18n("Upcoming new tasks"));
    tables[3]->clearColumns();
    tables[3]->addColumn(new TableColumnInfo(sc, "name"));
    tables[3]->addColumn(new TableColumnInfo(sc, "start"));
    tables[3]->addColumn(new TableColumnInfo(sc, "duration"));
    tables[3]->addColumn(new TableColumnInfo(sc, "resources"));
    tables[3]->addColumn(new TableColumnInfo(sc, "note"));
}

HTMLStatusReport::~HTMLStatusReport()
{
    for (int i = 0; i < tablesCount; i++)
        delete tables[i];
}

void HTMLStatusReport::setTable(int tabIdx, HTMLReportElement* tab)
{
    if (tables[tabIdx] && tables[tabIdx] != tab)
        delete tables[tabIdx];
    tables[tabIdx] = tab;
}

HTMLReportElement*
HTMLStatusReport::getTable(int tabIdx) const
{
    return tables[tabIdx];
}

bool
HTMLStatusReport::generate()
{
    if (!open())
        return FALSE;

    if (headline.isEmpty())
        headline = i18n("Status report for the period %1 to %2")
            .arg(time2user(start, timeFormat)).arg(time2user(end, timeFormat));

    generateHeader(i18n("Status Report"));

    for (int i = 0; i < tablesCount; ++i)
    {
        tables[i]->generate();
        s << "<br/>" << endl;
    }

    generateFooter();

    f.close();
    return TRUE;
}

