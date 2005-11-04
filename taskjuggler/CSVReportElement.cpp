/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>
#include <assert.h>

#include "CSVReportElement.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Report.h"
#include "Booking.h"
#include "BookingList.h"
#include "Utility.h"
#include "MacroTable.h"
#include "TableColumnFormat.h"
#include "TableLineInfo.h"
#include "TableColumnInfo.h"
#include "TableCellInfo.h"
#include "ReferenceAttribute.h"
#include "TextAttribute.h"
#include "CSVReport.h"
#include "UsageLimits.h"

#define KW(a) a

CSVReportElement::CSVReportElement(Report* r, const QString& df, int dl) :
   ReportElement(r, df, dl)
{
    fieldSeparator = ";";
}

void
CSVReportElement::generateHeader()
{
}

void
CSVReportElement::generateFooter()
{
}

void
CSVReportElement::generateTableHeader()
{
    bool first = TRUE;
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (!first)
            s() << fieldSeparator;
        else
            first = FALSE;

        if (columnFormat[(*it)->getName()])
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], 0, *it);
            (*this.*(columnFormat[(*it)->getName()]->genHeadLine1))
                (&tci);
        }
        else if ((*it)->getName() == "costs")
        {
            TJMH.errorMessage
                (i18n("'costs' has been deprecated. Use 'cost' instead."));
            return;
        }
        else
        {
            TJMH.errorMessage
                (i18n("Unknown Column '%1' for CSV Report")
                 .arg((*it)->getName()));
            return;
        }
    }

    if (!first)
        s() << endl;
}

void
CSVReportElement::generateLine(TableLineInfo* tli, int funcSel)
{
    setMacros(tli);

    bool first = TRUE;
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        TableCellInfo tci(columnFormat[(*it)->getName()], tli, *it);
        if (columnFormat[(*it)->getName()])
        {
            if (!first)
                s() << fieldSeparator;
            else
                first = FALSE;

            GenCellPtr gcf;
            switch (funcSel)
            {
                case 0:
                    gcf = columnFormat[(*it)->getName()]->genHeadLine1;
                    break;
                case 1:
                    gcf = columnFormat[(*it)->getName()]->genHeadLine2;
                    break;
                case 2:
                    gcf = columnFormat[(*it)->getName()]->genTaskLine1;
                    break;
                case 3:
                    gcf = columnFormat[(*it)->getName()]->genTaskLine2;
                    break;
                case 4:
                    gcf = columnFormat[(*it)->getName()]->genResourceLine1;
                    break;
                case 5:
                    gcf = columnFormat[(*it)->getName()]->genResourceLine2;
                    break;
                case 6:
                    gcf = columnFormat[(*it)->getName()]->genAccountLine1;
                    break;
                case 7:
                    gcf = columnFormat[(*it)->getName()]->genAccountLine2;
                    break;
                case 8:
                    gcf = columnFormat[(*it)->getName()]->genSummaryLine1;
                    break;
                case 9:
                    gcf = columnFormat[(*it)->getName()]->genSummaryLine2;
                    break;
            }
            if (gcf)
            {
                (*this.*(gcf))(&tci);
            }
        }
    }
    if (!first)
        s() << endl;
}

void
CSVReportElement::genCell(const QString& text, TableCellInfo* tci,
                          bool, bool filterText)
{
    QString cellText;
    if (tci->tli->ca1 == 0 ||
        !isHidden(tci->tli->ca1, tci->tci->getHideCellText()))
    {
        cellText = filterText ? filter(text) : text;
        if (tci->tli->ca1 && !tci->tci->getCellText().isEmpty())
        {
            QStringList* sl = new QStringList();
            sl->append(text);
            cellText = mt.expandReportVariable(tci->tci->getCellText(), sl);
            QString cellURL = mt.expandReportVariable(tci->tci->getCellURL(),
                                                      sl);
        }
    }
    s() << "\"" << cellText << "\"";
}

void
CSVReportElement::reportTaskLoad(double load, TableCellInfo* tci,
                                  const Interval& period)
{
    QString text;
    if (tci->tli->task->isActive(tci->tli->sc, period))
        text = scaledLoad(load, tci->tcf->realFormat);
    genCell(text, tci, FALSE);
}

void
CSVReportElement::reportResourceLoad(double load, TableCellInfo* tci,
                                      const Interval&)
{
    QString text;
    if (load > 0.0)
        text += scaledLoad(load, tci->tcf->realFormat);
    genCell(text, tci, FALSE);
}

void
CSVReportElement::reportCurrency(double value, TableCellInfo* tci, time_t)
{
    genCell(tci->tcf->realFormat.format(value, tci), tci, FALSE);
}

void
CSVReportElement::generateTitle(TableCellInfo* tci, const QString& str)
{
    QStringList* sl = new QStringList();
    sl->append(str);
    QString cellText;
    if (!tci->tci->getTitle().isEmpty())
    {
        cellText = mt.expandReportVariable(tci->tci->getTitle(), sl);
        if (!tci->tci->getSubTitle().isEmpty())
            cellText += " " + mt.expandReportVariable(tci->tci->getSubTitle(),
                                                      sl);
    }
    else
        cellText = str;
    cellText = filter(cellText);

    s() << "\"" << cellText << "\"";
}

void
CSVReportElement::generateSubTitle(TableCellInfo* tci, const QString&)
{
    tci->tci->increaseSubColumns();
}

void
CSVReportElement::generateRightIndented(TableCellInfo* tci, const QString& str)
{
    int topIndent = 0, maxDepth = 0;
    if (tci->tli->ca1->getType() == CA_Task)
    {
        if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
            topIndent = (tci->tli->ca2 != 0 ? 0 : 1) * maxDepthResourceList;
        maxDepth = maxDepthTaskList;
    }
    else if (tci->tli->ca1->getType() == CA_Resource)
    {
        if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
            topIndent = (tci->tli->ca2 != 0 ? 0 : 1) * maxDepthTaskList;
        maxDepth = maxDepthResourceList;
    }

    genCell(str + QString().fill(' ', topIndent + (maxDepth - 1 -
                                                   tci->tli->ca1->treeLevel())),
            tci, FALSE);
}

void
CSVReportElement::genHeadDefault(TableCellInfo* tci)
{
    generateTitle(tci, tci->tcf->getTitle());
}

void
CSVReportElement::genHeadCurrency(TableCellInfo* tci)
{

    generateTitle(tci, tci->tcf->getTitle() +
                  (!report->getProject()->getCurrency().isEmpty() ?
                   QString(" ") + report->getProject()->getCurrency() :
                   QString()));
}

void
CSVReportElement::genHeadDaily1(TableCellInfo* tci)
{
    bool first = TRUE;
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        int dom = dayOfMonth(day);
        mt.setMacro(new Macro(KW("day"), QString().sprintf("%02d", dom),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(day)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"),
                              QString().sprintf
                              ("%02d", quarterOfYear(day)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"),
                              QString().sprintf
                              ("%02d", weekOfYear(day, weekStartsMonday)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(day)),
                              defFileName, defFileLine));
        generateTitle(tci, time2user(day, "%Y-%m-%d"));
    }
}

void
CSVReportElement::genHeadDaily2(TableCellInfo*)
{
}

void
CSVReportElement::genHeadWeekly1(TableCellInfo* tci)
{
    // Generates the 1st header line for weekly calendar views.
    bool first = TRUE;
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end; )
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        int currMonth = monthOfWeek(week, weekStartsMonday);
        int left;
        time_t wi = sameTimeNextWeek(week);
        for (left = 1 ; wi < end &&
             monthOfWeek(wi, weekStartsMonday) == currMonth;
             wi = sameTimeNextWeek(wi))
            left++;

        mt.setMacro(new Macro(KW("day"), QString().sprintf
                              ("%02d", dayOfMonth(week)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"),
                              QString().sprintf
                              ("%02d", monthOfWeek(week, weekStartsMonday)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"),
                              QString().sprintf
                              ("%02d", quarterOfYear(week)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"),
                              QString().sprintf
                              ("%02d", weekOfYear(week, weekStartsMonday)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf
                              ("%04d", yearOfWeek(week, weekStartsMonday)),
                              defFileName, defFileLine));
        generateTitle(tci,
                      QString(i18n("%1 %2/Week %3"))
                      .arg(shortMonthName(monthOfWeek(week, weekStartsMonday)
                                          - 1))
                      .arg(yearOfWeek(week, weekStartsMonday))
                      .arg(weekOfYear(week, weekStartsMonday)));
        week = wi;
    }
}

void
CSVReportElement::genHeadWeekly2(TableCellInfo*)
{
}

void
CSVReportElement::genHeadMonthly1(TableCellInfo* tci)
{
    bool first = TRUE;
    bool wsm = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, wsm); week < end;
         week = sameTimeNextWeek(week))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        int woy = weekOfYear(week, wsm);
        mt.setMacro(new Macro(KW("day"), QString().sprintf
                              ("%02d", dayOfMonth(week)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"),
                              QString().sprintf("%02d",
                                                monthOfWeek(week, wsm)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"),
                              QString().sprintf
                              ("%02d", quarterOfYear(week)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"),
                              QString().sprintf("%02d", woy),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", yearOfWeek(week, wsm)),
                              defFileName, defFileLine));
        generateTitle(tci,
                      QString(i18n("%1 %2/Week %3"))
                      .arg(shortMonthName(monthOfWeek(week, wsm)
                                          - 1))
                      .arg(yearOfWeek(week, wsm)).arg(woy));
    }
}

void
CSVReportElement::genHeadMonthly2(TableCellInfo*)
{
}

void
CSVReportElement::genHeadQuarterly1(TableCellInfo* tci)
{
    static const char* qnames[] =
    {
        I18N_NOOP("1st Quarter"), I18N_NOOP("2nd Quarter"),
        I18N_NOOP("3rd Quarter"), I18N_NOOP("4th Quarter")
    };

    bool first = TRUE;
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        int qoy = quarterOfYear(quarter);
        mt.setMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(quarter)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(quarter)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"),
                              QString().sprintf("%d", qoy),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"),
                              QString().sprintf
                              ("%02d", weekOfYear(quarter, weekStartsMonday)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(quarter)),
                              defFileName, defFileLine));
        generateSubTitle(tci, i18n(qnames[qoy - 1]) + " " +
                         QString().sprintf("%d", ::year(quarter)));
    }
}

void
CSVReportElement::genHeadQuarterly2(TableCellInfo*)
{
}

void
CSVReportElement::genHeadYear(TableCellInfo* tci)
{
    bool first = TRUE;
    // Generates 1st header line of monthly calendar view.
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        mt.setMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(year)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(year)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"), "1",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", ::year(year)),
                              defFileName, defFileLine));
        generateTitle(tci, QString().sprintf("%d", ::year(year)));
    }
}

void
CSVReportElement::genCellEmpty(TableCellInfo* tci)
{
    genCell("", tci, TRUE);
}

void
CSVReportElement::genCellSequenceNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ?
            QString().sprintf("%d.", tci->tli->ca1->getSequenceNo()) :
            QString(""), tci, TRUE);
}

void
CSVReportElement::genCellNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? QString().sprintf("%d.", tci->tli->idxNo) :
            QString(""), tci, TRUE);
}

void
CSVReportElement::genCellHierarchNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ?
            tci->tli->ca1->getHierarchNo() : QString(""), tci, TRUE);
}

void
CSVReportElement::genCellIndex(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ?
            QString().sprintf("%d.", tci->tli->ca1->getIndex()) :
            QString(""), tci, TRUE);
}

void
CSVReportElement::genCellHierarchIndex(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ?
            tci->tli->ca1->getHierarchIndex() : QString(""), tci, TRUE);
}

void
CSVReportElement::genCellId(TableCellInfo* tci)
{
    genCell(tci->tli->ca1->getId(), tci, TRUE);
}

void
CSVReportElement::genCellName(TableCellInfo* tci)
{
    int lPadding = 0;
    if ((tci->tli->ca2 && (tci->tli->ca2->getType() == CA_Resource &&
          resourceSortCriteria[0] == CoreAttributesList::TreeMode)) ||
        (tci->tli->ca2 && tci->tli->ca2->getType() == CA_Task &&
         taskSortCriteria[0] == CoreAttributesList::TreeMode))
        for (const CoreAttributes* cp = tci->tli->ca2 ; cp != 0;
             cp = cp->getParent())
            lPadding++;

    QString text;
    if (tci->tli->specialName.isNull())
    {
        if (tci->tli->task)
            mt.setMacro(new Macro(KW("taskid"), tci->tli->task->getId(),
                                  defFileName, defFileLine));
        if (tci->tli->resource)
            mt.setMacro(new Macro(KW("resourceid"), tci->tli->resource->getId(),
                                  defFileName, defFileLine));
        if (tci->tli->account)
            mt.setMacro(new Macro(KW("accountid"), tci->tli->account->getId(),
                                  defFileName, defFileLine));

        if ((tci->tli->ca1->getType() == CA_Resource &&
             resourceSortCriteria[0] == CoreAttributesList::TreeMode) ||
            (tci->tli->ca1->getType() == CA_Task &&
             taskSortCriteria[0] == CoreAttributesList::TreeMode) ||
            (tci->tli->ca1->getType() == CA_Account &&
             accountSortCriteria[0] == CoreAttributesList::TreeMode))
        {
            lPadding += tci->tli->ca1->treeLevel();
        }
        text = QString().fill(' ', lPadding) + tci->tli->ca1->getName();
    }
    else
        text = tci->tli->specialName;
    genCell(text, tci, TRUE);
}

void
CSVReportElement::genCellStart(TableCellInfo* tci)
{
    if (!tci->tli->task->isStartOk(tci->tli->sc))
        tci->setBgColor(colors.getColor("error"));
    genCell(time2user(tci->tli->task->getStart(tci->tli->sc), timeFormat),
            tci, FALSE);
}

void
CSVReportElement::genCellEnd(TableCellInfo* tci)
{
    if (!tci->tli->task->isEndOk(tci->tli->sc))
        tci->setBgColor(colors.getColor("error"));
    genCell(time2user(tci->tli->task->getEnd(tci->tli->sc) +
                      (tci->tli->task->isMilestone() ? 1 : 0), timeFormat),
            tci, FALSE);
}

#define GCMMSE(a) \
void \
CSVReportElement::genCell##a(TableCellInfo* tci) \
{ \
    genCell(tci->tli->task->get##a(tci->tli->sc) == 0 ? QString() : \
            time2user(tci->tli->task->get##a(tci->tli->sc), timeFormat), \
            tci, FALSE); \
}

GCMMSE(MinStart)
GCMMSE(MaxStart)
GCMMSE(MinEnd)
GCMMSE(MaxEnd)

#define GCSEBUFFER(a) \
void \
CSVReportElement::genCell##a##Buffer(TableCellInfo* tci) \
{ \
    genCell(QString().sprintf \
            ("%3.0f", tci->tli->task->get##a##Buffer(tci->tli->sc)), \
            tci, FALSE); \
}

GCSEBUFFER(Start)
GCSEBUFFER(End)

void
CSVReportElement::genCellStartBufferEnd(TableCellInfo* tci)
{
    genCell(time2user(tci->tli->task->getStartBufferEnd
                         (tci->tli->sc), timeFormat), tci, FALSE);
}

void
CSVReportElement::genCellEndBufferStart(TableCellInfo* tci)
{
    genCell(time2user(tci->tli->task->getStartBufferEnd
                      (tci->tli->sc) + 1, timeFormat), tci, FALSE);
}

void
CSVReportElement::genCellDuration(TableCellInfo* tci)
{
    genCell(scaledLoad(tci->tli->task->getCalcDuration(tci->tli->sc),
                       tci->tcf->realFormat),
            tci, FALSE);
}

void
CSVReportElement::genCellEffort(TableCellInfo* tci)
{
    double val = 0.0;
    if (tci->tli->ca1->getType() == CA_Task)
    {
        val = tci->tli->task->getLoad(tci->tli->sc, Interval(start, end),
                                      tci->tli->resource);
    }
    else if (tci->tli->ca1->getType() == CA_Resource)
    {
        val = tci->tli->resource->getLoad(tci->tli->sc, Interval(start, end),
                                          AllAccounts, tci->tli->task);
    }

    generateRightIndented(tci, scaledLoad(val, tci->tcf->realFormat));
}

void
CSVReportElement::genCellFreeLoad(TableCellInfo* tci)
{
    double val = 0.0;
    if (tci->tli->ca1->getType() == CA_Resource)
    {
        val = tci->tli->resource->getAvailableWorkLoad
            (tci->tli->sc, Interval(start, end));
    }

    generateRightIndented(tci, scaledLoad(val, tci->tcf->realFormat));
}

void
CSVReportElement::genCellUtilization(TableCellInfo* tci)
{
    double val = 0.0;
    if (tci->tli->ca1->getType() == CA_Resource)
    {
        double load =
            tci->tli->resource->getLoad(tci->tli->sc, Interval(start, end));
        double availableLoad =
            tci->tli->resource->getAvailableWorkLoad
            (tci->tli->sc, Interval(start, end));

        val = 100.0 / (1.0 + availableLoad / load);
    }

    generateRightIndented(tci, QString().sprintf("%.1f%%", val));
}

void
CSVReportElement::genCellCriticalness(TableCellInfo* tci)
{
    generateRightIndented
        (tci, scaledLoad(tci->tli->task->getCriticalness(tci->tli->sc),
                         tci->tcf->realFormat));
}

void
CSVReportElement::genCellPathCriticalness(TableCellInfo* tci)
{
    generateRightIndented
        (tci, scaledLoad(tci->tli->task->getPathCriticalness(tci->tli->sc),
                         tci->tcf->realFormat));
}

void
CSVReportElement::genCellProjectId(TableCellInfo* tci)
{
    genCell(tci->tli->task->getProjectId() + " (" +
            report->getProject()->getIdIndex(tci->tli->task->
                                             getProjectId()) + ")", tci,
            TRUE);
}

void
CSVReportElement::genCellProjectIDs(TableCellInfo* tci)
{
    genCell(tci->tli->resource->getProjectIDs(tci->tli->sc,
                                              Interval(start, end)), tci, TRUE);
}

void
CSVReportElement::genCellResources(TableCellInfo* tci)
{
    QString text;
    for (ResourceListIterator rli(tci->tli->task->
                                  getBookedResourcesIterator(tci->tli->sc));
        *rli != 0; ++rli)
    {
        if (!text.isEmpty())
            text += ", ";

        text += (*rli)->getName();
    }
    genCell(text, tci, FALSE);
}

void
CSVReportElement::genCellResponsible(TableCellInfo* tci)
{
    if (tci->tli->task->getResponsible())
        genCell(tci->tli->task->getResponsible()->getName(),
                tci, TRUE);
    else
        genCell("", tci, TRUE);
}

void
CSVReportElement::genCellText(TableCellInfo* tci)
{
    if (tci->tcf->getId() == "note")
    {
        if (tci->tli->task->getNote().isEmpty())
            genCell("", tci, TRUE);
        else
            genCell(tci->tli->task->getNote(), tci, TRUE);
        return;
    }

    const TextAttribute* ta = (const TextAttribute*)
        tci->tli->ca1->getCustomAttribute(tci->tcf->getId());
    if (!ta || ta->getText().isEmpty())
        genCell("", tci, TRUE);
    else
        genCell(ta->getText(), tci, TRUE);
}

void
CSVReportElement::genCellStatusNote(TableCellInfo* tci)
{
    if (tci->tli->task->getStatusNote(tci->tli->sc).isEmpty())
        genCell("", tci, TRUE);
    else
        genCell(tci->tli->task->getStatusNote(tci->tli->sc),
                tci, TRUE);
}

void
CSVReportElement::genCellCost(TableCellInfo* tci)
{
    double val = 0.0;
    if (tci->tli->ca1->getType() == CA_Task)
    {
        val = tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                       Cost, tci->tli->resource);
    }
    else if (tci->tli->ca1->getType() == CA_Resource)
    {
        val = tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                        Cost, tci->tli->task);
    }
    generateRightIndented(tci, tci->tcf->realFormat.format(val, tci));
}

void
CSVReportElement::genCellRevenue(TableCellInfo* tci)
{
    double val = 0.0;
    if (tci->tli->ca1->getType() == CA_Task)
    {
        val = tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                       Revenue, tci->tli->resource);
    }
    else if (tci->tli->ca1->getType() == CA_Resource)
    {
        val = tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                        Revenue, tci->tli->task);
    }
    generateRightIndented(tci, tci->tcf->realFormat.format(val, tci));
}

void
CSVReportElement::genCellProfit(TableCellInfo* tci)
{
    double val = 0.0;
    if (tci->tli->ca1->getType() == CA_Task)
    {
        val = tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                    Revenue, tci->tli->resource) -
            tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                  Cost, tci->tli->resource);
    }
    else if (tci->tli->ca1->getType() == CA_Resource)
    {
        val = tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                        Revenue, tci->tli->task) -
            tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                      Cost, tci->tli->task);
    }
    generateRightIndented(tci, tci->tcf->realFormat.format(val, tci));
}

void
CSVReportElement::genCellPriority(TableCellInfo* tci)
{
    genCell(QString().sprintf("%d", tci->tli->task->getPriority()),
            tci, TRUE);
}

void
CSVReportElement::genCellFlags(TableCellInfo* tci)
{
    FlagList allFlags = tci->tli->ca1->getFlagList();
    QString flagStr;
    for (QStringList::Iterator it = allFlags.begin();
         it != allFlags.end(); ++it)
    {
        if (it != allFlags.begin())
            flagStr += ", ";
        flagStr += *it;
    }
    genCell(flagStr, tci, TRUE);
}

void
CSVReportElement::genCellCompleted(TableCellInfo* tci)
{
    if (tci->tli->task->getCompletionDegree(tci->tli->sc) ==
        tci->tli->task->getCalcedCompletionDegree(tci->tli->sc))
    {
        genCell(QString("%1%").arg((int) tci->tli->task->
                                   getCompletionDegree(tci->tli->sc)),
                tci, FALSE);
    }
    else
    {
        genCell(QString("%1% (%2%)")
                .arg((int) tci->tli->task->getCompletionDegree(tci->tli->sc))
                .arg((int) tci->tli->task->
                     getCalcedCompletionDegree(tci->tli->sc)),
                tci, FALSE);
    }
}

void
CSVReportElement::genCellStatus(TableCellInfo* tci)
{
    genCell(tci->tli->task->getStatusText(tci->tli->sc), tci, FALSE);
}

void
CSVReportElement::genCellReference(TableCellInfo* tci)
{
    if (tci->tcf->getId() == "reference")
    {
        if (tci->tli->task->getReference().isEmpty())
            genCell("", tci, TRUE);
        else
        {
            QString text =tci->tli->task->getReference();
            if (tci->tli->task->getReferenceLabel().isEmpty())
                text += filter(tci->tli->task->getReference());
            else
                text += filter(tci->tli->task->getReferenceLabel());
            genCell(text, tci, TRUE, FALSE);
        }
        return;
    }

    const ReferenceAttribute* ra =  (const ReferenceAttribute*)
        tci->tli->ca1->getCustomAttribute(tci->tcf->getId());
    if (!ra || ra->getURL().isEmpty())
        genCell("", tci, TRUE);
    else
    {
        QString text = ra->getURL();
        if (ra->getLabel().isEmpty())
            text += filter(ra->getURL());
        else
            text += filter(ra->getLabel());
        genCell(text, tci, TRUE, FALSE);
    }
}

void
CSVReportElement::genCellScenario(TableCellInfo* tci)
{
    genCell(report->getProject()->getScenarioName(tci->tli->sc), tci, FALSE);
}

#define GCDEPFOL(a, b) \
void \
CSVReportElement::genCell##a(TableCellInfo* tci) \
{ \
    QString text; \
    for (TaskListIterator it(tci->tli->task->get##b##Iterator()); *it != 0; \
         ++it) \
    { \
        if (!text.isEmpty()) \
            text += ", "; \
        text += (*it)->getId(); \
    } \
    genCell(text, tci, TRUE); \
}

GCDEPFOL(Depends, Previous)
GCDEPFOL(Follows, Followers)

QColor
CSVReportElement::selectTaskBgColor(TableCellInfo* tci, double load,
                                     const Interval& period, bool daily)
{
    QColor bgCol;
    if (tci->tli->task->isActive(tci->tli->sc, period) &&
        ((tci->tli->resource != 0 && load > 0.0) || tci->tli->resource == 0))
    {
        if (tci->tli->task->isCompleted(tci->tli->sc, period.getEnd() - 1))
        {
            if (tci->tli->ca2 == 0)
                bgCol = colors.getColor("completed");
            else
                bgCol = colors.getColor("completed").light(130);
        }
        else
        {
            if (tci->tli->ca2 == 0 &&
                !tci->tli->task->isBuffer(tci->tli->sc, period))
            {
                bgCol = colors.getColor("booked");
            }
            else
            {
                bgCol = colors.getColor("booked").light(130);
            }
        }
    }
    else if (period.contains(report->getProject()->getNow()))
    {
        bgCol = colors.getColor("today");
    }
    else if (daily && (isWeekend(period.getStart()) ||
                       report->getProject()->isVacation(period.getStart())))
    {
            bgCol = colors.getColor("vacation");
    }

    return bgCol;
}

QColor
CSVReportElement::selectResourceBgColor(TableCellInfo* tci, double load,
                                         const Interval& period, bool daily)
{
    QColor bgCol;
    if (load > tci->tli->resource->getMinEffort() *
            tci->tli->resource->getEfficiency())
    {
        if (tci->tli->ca2 == 0)
        {
            bgCol = colors.getColor("booked");
        }
        else
        {
            if (tci->tli->task->isCompleted(tci->tli->sc, period.getEnd() - 1))
                bgCol = colors.getColor("completed").light(130);
            else
                bgCol = colors.getColor("booked").light(130);
        }
    }
    else if (period.contains(report->getProject()->getNow()))
    {
        bgCol = colors.getColor("today");
    }
    else if (daily && (isWeekend(period.getStart()) ||
                       report->getProject()->isVacation(period.getStart()) ||
                       tci->tli->resource->hasVacationDay(period.getStart())))
    {
            bgCol = colors.getColor("vacation");
    }

    return bgCol;
}

void
CSVReportElement::genCellTaskFunc(TableCellInfo* tci,
                                  time_t (*beginOfT)(time_t),
                                  time_t (*sameTimeNextT)(time_t))
{
    bool first = TRUE;
    for (time_t t = (*beginOfT)(start); t < end; t = (*sameTimeNextT)(t))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        Interval period = Interval(t, sameTimeNextT(t) - 1);
        double load = tci->tli->task->getLoad(tci->tli->sc, period,
                                              tci->tli->resource);
        reportTaskLoad(load, tci, period);
    }
}

void
CSVReportElement::genCellResourceFunc(TableCellInfo* tci,
                                      time_t (*beginOfT)(time_t),
                                      time_t (*sameTimeNextT)(time_t))
{
    bool first = TRUE;
    for (time_t t = beginOfT(start); t < end; t = sameTimeNextT(t))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        Interval period = Interval(t, sameTimeNextT(t) - 1);
        double load = tci->tli->resource->getLoad(tci->tli->sc, period,
                                                  AllAccounts, tci->tli->task);
        reportResourceLoad(load, tci, period);
    }
}

void
CSVReportElement::genCellAccountFunc(TableCellInfo* tci,
                                     time_t (*beginOfT)(time_t),
                                     time_t (*sameTimeNextT)(time_t))
{
    bool first = TRUE;
    tci->tcf->realFormat = currencyFormat;
    for (time_t t = beginOfT(start); t < end; t = sameTimeNextT(t))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        double volume = tci->tli->account->
            getVolume(tci->tli->sc, Interval(t, sameTimeNextT(t) - 1));
        if ((accountSortCriteria[0] == CoreAttributesList::TreeMode &&
             tci->tli->account->isRoot()) ||
            (accountSortCriteria[0] != CoreAttributesList::TreeMode))
            tci->tci->addToSum(tci->tli->sc, time2ISO(t), volume);
        reportCurrency(volume, tci, t);
    }
}

void
CSVReportElement::genCellDailyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, midnight, sameTimeNextDay);
}

void
CSVReportElement::genCellDailyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, midnight, sameTimeNextDay);
}

void
CSVReportElement::genCellDailyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, midnight, sameTimeNextDay);
}

void
CSVReportElement::genCellWeeklyTask(TableCellInfo* tci)
{
    bool first = TRUE;
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        Interval period = Interval(week).firstWeek(weekStartsMonday);
        double load = tci->tli->task->getLoad(tci->tli->sc, period,
                                              tci->tli->resource);
        reportTaskLoad(load, tci, period);
    }
}

void
CSVReportElement::genCellWeeklyResource(TableCellInfo* tci)
{
    bool first = TRUE;
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        Interval period = Interval(week).firstWeek(weekStartsMonday);
        double load = tci->tli->resource->getLoad(tci->tli->sc, period,
                                                  AllAccounts, tci->tli->task);
        reportResourceLoad(load, tci, period);
    }
}

void
CSVReportElement::genCellWeeklyAccount(TableCellInfo* tci)
{
    bool first = TRUE;
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        if (first)
            first = FALSE;
        else
            s() << fieldSeparator;

        double volume = tci->tli->account->
            getVolume(tci->tli->sc, Interval(week).firstWeek(weekStartsMonday));
        if ((accountSortCriteria[0] == CoreAttributesList::TreeMode &&
             tci->tli->account->isRoot()) ||
            (accountSortCriteria[0] != CoreAttributesList::TreeMode))
            tci->tci->addToSum(tci->tli->sc, time2ISO(week), volume);
        reportCurrency(volume, tci, week);
    }
}

void
CSVReportElement::genCellMonthlyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, beginOfMonth, sameTimeNextMonth);
}

void
CSVReportElement::genCellMonthlyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, beginOfMonth, sameTimeNextMonth);
}

void
CSVReportElement::genCellMonthlyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, beginOfMonth, sameTimeNextMonth);
}

void
CSVReportElement::genCellQuarterlyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, beginOfQuarter, sameTimeNextQuarter);
}

void
CSVReportElement::genCellQuarterlyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, beginOfQuarter, sameTimeNextQuarter);
}

void
CSVReportElement::genCellQuarterlyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, beginOfQuarter, sameTimeNextQuarter);
}

void
CSVReportElement::genCellYearlyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, beginOfYear, sameTimeNextYear);
}

void
CSVReportElement::genCellYearlyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, beginOfYear, sameTimeNextYear);
}

void
CSVReportElement::genCellYearlyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, beginOfYear, sameTimeNextYear);
}

void
CSVReportElement::genCellResponsibilities(TableCellInfo* tci)
{
    QString text;
    for (TaskListIterator it(report->getProject()->getTaskListIterator());
         *it != 0; ++it)
    {
        if ((*it)->getResponsible() == tci->tli->resource)
        {
            if (!text.isEmpty())
                text += ", ";
            text += (*it)->getId();
        }
    }
    genCell(text, tci, TRUE);
}

void
CSVReportElement::genCellSchedule(TableCellInfo*)
{
}

#define GCEFFORT(a) \
void \
CSVReportElement::genCell##a##Effort(TableCellInfo* tci) \
{ \
    genCell(tci->tcf->realFormat.format \
            (tci->tli->resource->get##a##Effort(), FALSE), \
            tci, TRUE); \
}

GCEFFORT(Min)

void
CSVReportElement::genCellMaxEffort(TableCellInfo* tci)
{
    genCell(tci->tcf->realFormat.format
            (tci->tli->resource->getLimits() ?
             tci->tli->resource->getLimits()->getDailyMax() : 0, FALSE),
            tci, TRUE);
}

void
CSVReportElement::genCellEfficiency(TableCellInfo* tci)
{
    genCell(tci->tcf->realFormat.format(tci->tli->resource->getEfficiency(),
                                        tci), tci, TRUE);
}

void
CSVReportElement::genCellRate(TableCellInfo* tci)
{
    genCell(tci->tcf->realFormat.format(tci->tli->resource->getRate(), tci),
            tci, TRUE);
}

void
CSVReportElement::genCellKotrusId(TableCellInfo* tci)
{
    genCell(tci->tli->resource->getKotrusId(), tci, TRUE);
}

void
CSVReportElement::genCellTotal(TableCellInfo* tci)
{
    double value = tci->tli->account->getVolume(tci->tli->sc,
                                                Interval(start, end));
    if (tci->tli->account->isLeaf())
        tci->tci->addToSum(tci->tli->sc, "total", value);
    genCell(tci->tcf->realFormat.format(value, tci), tci, FALSE);
}

void
CSVReportElement::genCellSummary(TableCellInfo* tci)
{
    QMap<QString, double>::ConstIterator it;
    const QMap<QString, double>* sum = tci->tci->getSum();
    assert(sum != 0);

    uint sc = tci->tli->sc;
    double val = 0.0;
    if (sum[sc].begin() != sum[sc].end())
    {
        for (it = sum[sc].begin(); it != sum[sc].end(); ++it)
        {
            if (accumulate)
                val += *it;
            else
                val = *it;
            genCell(tci->tcf->realFormat.format(val, tci), tci, FALSE);
        }
    }
    else
    {
        // The column counter is not set in all cases. These are always single
        // column cases.
        if (tci->tci->getSubColumns() > 0)
            for (uint col = 0; col < tci->tci->getSubColumns(); ++col)
                genCell(tci->tcf->realFormat.format(0.0, tci), tci, FALSE);
        else
            genCell(tci->tcf->realFormat.format(0.0, tci), tci, FALSE);
    }
}

