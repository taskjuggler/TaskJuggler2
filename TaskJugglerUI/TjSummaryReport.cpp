/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TjSummaryReport.h"

#include <qtextbrowser.h>
#include <qlayout.h>

#include <klocale.h>

#include "Project.h"
#include "Scenario.h"
#include "Resource.h"
#include "Account.h"
#include "Task.h"

TjSummaryReport::TjSummaryReport(QWidget* p, ReportManager* m,
                                 const Project* pr,
                                 const QString& n) :
    TjUIReportBase(p, m, 0, n), project(pr)
{
    QVBoxLayout* hl = new QVBoxLayout(this, 1);
    hl->setAutoAdd(true);
    textBrowser = new QTextBrowser(this);
    textBrowser->setTextFormat(Qt::RichText);
    textBrowser->setReadOnly(true);
}

bool
TjSummaryReport::generateReport()
{
    QString text;

    text = i18n("<p><h1>Summary for Project %1 (Version %2)</h1></p><hr/>")
        .arg(project->getName())
        .arg(project->getVersion());

    int disabledScenarios = 0;
    for (ScenarioListIterator sli(project->getScenarioIterator()); *sli; ++sli)
        if (!(*sli)->getEnabled())
            disabledScenarios++;
    text += "<table>";
    text += i18n("<tr><td><b>Scenarios:</b></td><td>%1 (%2 disabled)</td></tr>")
        .arg(project->getMaxScenarios())
        .arg(disabledScenarios);

    int groups= 0;
    int workers = 0;
    int other = 0;
    for (ResourceListIterator rli(project->getResourceListIterator()); *rli;
         ++rli)
        if ((*rli)->hasSubs())
            groups++;
        else if ((*rli)->getEfficiency() > 0.0)
            workers++;
        else
            other++;
    text += i18n("<tr><td><b>Resources:</b></td>"
                 "<td>%1 (%2 Groups, %3 Workers, %4 Other)</td></tr>")
        .arg(project->resourceCount())
        .arg(groups).arg(workers).arg(other);

    int summaryAccounts = 0;
    int cost = 0;
    int revenue = 0;
    for (AccountListIterator ali(project->getAccountListIterator()); *ali;
         ++ali)
        if ((*ali)->hasSubs())
            summaryAccounts++;
        else if ((*ali)->getAcctType() == Cost)
            cost++;
        else
            revenue++;
    text += i18n("<tr><td><b>Accounts:</b></td>"
                 "<td>%1 (%2 Summary Accounts, %3 Cost Accounts, "
                 "%4 Revenue Accounts)</td></tr>")
        .arg(project->accountCount())
        .arg(summaryAccounts).arg(cost).arg(revenue);

    int containers = 0;
    int leafs = 0;
    int milestones = 0;
    for (TaskListIterator tli(project->getTaskListIterator()); *tli; ++tli)
        if ((*tli)->hasSubs())
            containers++;
        else if ((*tli)->isMilestone())
            milestones++;
        else
            leafs++;
    text += i18n("<tr><td><b>Tasks:</b></td><td>%1 "
                 "(%2 Containers, %3 Milestones, %4 Leaves)</td></tr>"
                "</table><hr/>")
        .arg(project->taskCount())
        .arg(containers).arg(milestones).arg(leafs);

    textBrowser->setText(text);

    return true;
}

void
TjSummaryReport::print()
{
}

void
TjSummaryReport::show()
{
    QWidget::show();
}

void
TjSummaryReport::hide()
{
    QWidget::hide();
}

#include "TjSummaryReport.moc"

