/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include "HTMLReportElement.h"
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
#include "HTMLReport.h"

#define KW(a) a

HTMLReportElement::HTMLReportElement(Report* r, const QString& df, int dl) :
   ReportElement(r, df, dl)
{
}

void
HTMLReportElement::generateHeader()
{
    if (!rawHead.isEmpty())
        s() << rawHead << endl;
    if (!headline.isEmpty())
        s() << "<h3>" << htmlFilter(headline) << "</h3>" << endl;
    if (!caption.isEmpty())
        s() << "<p>" << htmlFilter(caption) << "</p>" << endl;
}

void
HTMLReportElement::generateFooter()
{
    if (!rawTail.isEmpty())
        s() << rawTail << endl;
}

void
HTMLReportElement::generateTableHeader()
{
    // Header line 1
    s() << "<table align=\"center\" cellpadding=\"2\""
        << "style=\"background-color:#000000\"";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_table\"";
    s() << ">" << endl;
    s() << " <thead>" << endl 
        << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" << colors.getColorName("header") << "; "
        << "font-size:110%; font-weight:bold; text-align:center\"";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_header_row\"";
    s() << ">" << endl;
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
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
                (i18n("Unknown Column '%1' for HTML Report")
                 .arg((*it)->getName()));
            return;
        }
    }
    s() << "  </tr>" << endl;

    // Header line 2
    bool first = TRUE;
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()])
            if (columnFormat[(*it)->getName()]->genHeadLine2)
            {
                if (first)
                {
                    s() << "  <tr";
                    if (((HTMLReport*) report)->hasStyleSheet())
                        s() << " class=\"tj_header_row\"";
                    s() << ">" << endl;
                    first = FALSE;
                }

                TableCellInfo tci(columnFormat[(*it)->getName()], 0, *it);
                (*this.*(columnFormat[(*it)->getName()]->genHeadLine2))
                    (&tci);
            }
    }
    if (!first)
        s() << "  </tr>" << endl;

    s() << " </thead>\n" << endl;
}

void
HTMLReportElement::generateLine(TableLineInfo* tli, int funcSel)
{
    setMacros(tli);

    s() << "  <tr valign=\"middle\"";
    if (tli->bgCol.isValid() || tli->boldText || tli->fontFactor != 100)
    {
       s() << " style=\"";
       if (tli->bgCol.isValid())
           s() << "background-color:" << tli->bgCol.name() << "; ";
       if (tli->boldText)    
           s() << "font-weight:bold; ";
       if (tli->fontFactor != 100)
           s() << "font-size:" << QString("%1").arg(tli->fontFactor) << "%; ";
       s() << "\"";
    }
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_row\"";
    s() << ">" << endl;
    
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        TableCellInfo tci(columnFormat[(*it)->getName()], tli, *it);
        if (columnFormat[(*it)->getName()])
        {
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
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::genCell(const QString& text, TableCellInfo* tci, 
                           bool multi, bool filter)
{
    if (!multi)
        tci->setFontFactor(90);

    s() << "   <td";
    if (tci->tcf->noWrap)
        s() << " nowrap=\"nowrap\"";
    if (tci->getRows() != 1 || (multi && scenarios.count() > 1))
        s() << " rowspan=\"" << QString("%1")
            .arg(tci->getRows() != 1 ?
                 tci->getRows() : scenarios.count()) << "\"";
    if (tci->getColumns() != 1)
        s() << " colspan=\"" << QString("%1").arg(tci->getColumns()) << "\"";
    if (!tci->getStatusText().isEmpty())
    {
        s() << " onmouseover=\"status='" << tci->getStatusText() 
            << "';return true;\"";
    }
    if (!tci->tcf->hAlign.isEmpty() || 
        !tci->getHAlign().isEmpty() ||
        tci->getLeftPadding() > 0 ||
        tci->getRightPadding() > 0 ||
        tci->getBgColor().isValid() ||
        tci->getFontFactor() != 100 ||
        tci->getBoldText() ||
        tci->tcf->fontFactor != 100)
    {
        s() << " style=\"";
        if (tci->getBgColor().isValid())
            s() << "background-color:" << tci->getBgColor().name() << "; ";
        if (!tci->getHAlign().isEmpty())
            s() << "text-align:" << tci->getHAlign() << "; ";
        else if (!tci->tcf->hAlign.isEmpty())
            s() << "text-align:" << tci->tcf->hAlign << "; ";
        if (tci->getLeftPadding() > 0)
            s() << "padding-left:" <<
                QString("%1").arg(tci->getLeftPadding()) << "; ";
        if (tci->getRightPadding() > 0)
            s() << "padding-right:" <<
                QString("%1").arg(tci->getRightPadding()) << "; ";
        if (tci->getBoldText())
            s() << "font-weight:bold; ";
        if (tci->getFontFactor() != 100 || tci->tcf->fontFactor != 100)
            s() << "font-size:" 
                << QString("%1").arg(tci->getFontFactor() *
                                     tci->tcf->fontFactor / 100)
                << "%; ";
        s() << "\"";
    }
    QString cellText = filter ? htmlFilter(text) : text;
    if (tci->tli->ca1 && !tci->tci->getCellText().isEmpty())
    {
        QStringList* sl = new QStringList();
        sl->append(text);
        mt.pushArguments(sl);
        cellText = mt.expand(tci->tci->getCellText());
        QString cellURL = mt.expand(tci->tci->getCellURL());
        if (!cellURL.isEmpty())
        cellText = QString("<a href=\"") + cellURL
            + "\">" + htmlFilter(cellText) + "</a>";

        mt.popArguments();
    }
    if (cellText.isEmpty())
        cellText = "&nbsp;";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_cell\"";
    s() << ">";
    if (!tci->getToolTipText().isEmpty())
    {
        s() << "<div id=\"" << tci->getToolTipID() 
            << "\" class=\"tj_tooltip\" style=\"visibility:hidden\">"
            << tci->getToolTipText()
            << "</div>";
    }

    s() << cellText << "</td>" << endl;
}

void
HTMLReportElement::reportTaskLoad(double load, TableCellInfo* tci,
                                  const Interval& period)
{
    QString text;
    if (tci->tli->task->isActive(tci->tli->sc, period))
    {
        if (tci->tli->task->isContainer())
        {
            QString pre, post;
            if (period.contains(tci->tli->task->getStart(tci->tli->sc)))
                pre = "v=";
            if (period.contains(tci->tli->task->getEnd(tci->tli->sc)))
                post += "=v";
            if (load > 0.0 && barLabels != BLT_EMPTY)
                text = scaledLoad(load, tci);
            else if (pre.isEmpty() && post.isEmpty())
                text = "==";
            else if (!pre.isEmpty() && !post.isEmpty())
            {
                pre = post = "v";
                text = "=";
            }
            text = pre + text + post;
            tci->setBoldText(true);
        }
        else
        {
            if (tci->tli->task->isMilestone())
            {
                text += "<>";
                tci->setBoldText(true);
            }
            else
            {
                QString pre, post;
                if (period.contains(tci->tli->task->
                                    getStart(tci->tli->sc)))
                    pre = "[=";
                if (period.contains(tci->tli->task->
                                    getEnd(tci->tli->sc)))
                    post = "=]";
                if (!pre.isEmpty() && !post.isEmpty())
                {
                    pre = "[";
                    post = "]";
                }
                if (load > 0.0 && barLabels != BLT_EMPTY)
                    text = scaledLoad(load, tci);
                else if (pre.isEmpty() && post.isEmpty())
                    text = "==";
                else if (pre == "[")
                   text = "="; 
                text = pre + text + post;
            }
        }
        tci->setHAlign("center");
        tci->setStatusText(time2user(period.getStart(), "%Y-%m-%d / [") +
                           tci->tli->task->getId() + "] " +
                           htmlFilter(tci->tli->task->getName()));
    }
    else
    {
        tci->setStatusText("");
    }
    genCell(text, tci, FALSE);
}

void
HTMLReportElement::reportResourceLoad(double load, TableCellInfo* tci,
                                      const Interval& period)
{
    QString text;
    if (load > 0.0)
    {
        if (barLabels != BLT_EMPTY)
            text += scaledLoad(load, tci);
        if (tci->tli->resource->hasSubs())
            tci->setBoldText(true);
        tci->setHAlign("center");
        tci->setStatusText(time2user(period.getStart(), "%Y-%m-%d / [") +
                           tci->tli->resource->getId() + "] " +
                           htmlFilter(tci->tli->resource->getName()));
    }
    else
    {
        tci->setStatusText("");
    }
    genCell(text, tci, FALSE);
}

void
HTMLReportElement::reportCurrency(double value, TableCellInfo* tci, 
                                  time_t iv_start)
{
    tci->setStatusText(time2user(iv_start, "%Y-%m-%d / [") +
                       tci->tli->account->getId() + "] " +
                       htmlFilter(tci->tli->account->getName()));
    genCell(tci->tcf->realFormat.format(value, tci), tci, FALSE);
}

void
HTMLReportElement::generateTitle(TableCellInfo* tci, const QString& str)
{
    QStringList* sl = new QStringList();
    sl->append(str);
    mt.pushArguments(sl);
    QString cellText;
    if (!tci->tci->getTitle().isEmpty())
        cellText = mt.expand(tci->tci->getTitle());
    else
        cellText = str;
    cellText = htmlFilter(cellText);
    QString cellURL = mt.expand(tci->tci->getTitleURL());
    if (!cellURL.isEmpty())
        cellText = QString("<a href=\"") + cellURL
            + "\">" + cellText + "</a>";
    mt.popArguments();

    s() << cellText;
}

void
HTMLReportElement::generateSubTitle(TableCellInfo* tci, const QString& str)
{
    QStringList* sl = new QStringList();
    sl->append(str);
    mt.pushArguments(sl);
    QString cellText;
    if (!tci->tci->getTitle().isEmpty())
        cellText = mt.expand(tci->tci->getTitle());
    else
        cellText = str;
    cellText = htmlFilter(cellText);
    QString cellURL = mt.expand(tci->tci->getSubTitleURL());
    if (!cellURL.isEmpty())
        cellText = QString("<a href=\"") + cellURL
            + "\">" + cellText + "</a>";
    mt.popArguments();

    s() << cellText;
}

void
HTMLReportElement::generateRightIndented(TableCellInfo* tci, const QString& str)
{
    int topIndent = 0, subIndent = 0, maxDepth = 0;
    if (tci->tli->ca1->getType() == CA_Task)
    {
        if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
            subIndent = tci->tli->ca2 == 0 ? 8 : 5;
        if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
            topIndent = (tci->tli->ca2 != 0 ? 0 : 5) * maxDepthResourceList;
        maxDepth = maxDepthTaskList;
    }
    else if (tci->tli->ca1->getType() == CA_Resource)
    {
        if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
            subIndent = tci->tli->ca2 == 0 ? 8 : 5;
        if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
            topIndent = (tci->tli->ca2 != 0 ? 0 : 5) * maxDepthTaskList;
        maxDepth = maxDepthResourceList;
    }

    tci->setRightPadding(2 + topIndent + 
                         (maxDepth - 1 - tci->tli->ca1->treeLevel()) * 
                         subIndent);
    genCell(str, tci, FALSE);
}

void
HTMLReportElement::genHeadDefault(TableCellInfo* tci)
{
    s() << "   <td rowspan=\"2\"";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_header_cell\"";
    s() << ">";
    generateTitle(tci, tci->tcf->getTitle());
    s() << "</td>" << endl;
}

void
HTMLReportElement::genHeadCurrency(TableCellInfo* tci)
{

    s() << "   <td rowspan=\"2\"";
    if (((HTMLReport*) report)->hasStyleSheet())
        s() << " class=\"tj_header_cell\"";
    s() << ">";
    generateTitle(tci, tci->tcf->getTitle() +
                  (!report->getProject()->getCurrency().isEmpty() ?
                   QString(" ") + report->getProject()->getCurrency() :
                   QString()));
    s() << "</td>" << endl;
}

void
HTMLReportElement::genHeadDaily1(TableCellInfo* tci)
{
    // Generates the 1st header line for daily calendar views.
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextMonth(beginOfMonth(day)))
    {
        int left = daysLeftInMonth(day);
        if (left > daysBetween(day, end))
            left = daysBetween(day, end);
        s() << "   <td colspan=\""
            << QString().sprintf("%d", left) << "\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">"; 
        mt.setMacro(new Macro(KW("day"), "01",
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
        generateTitle(tci, monthAndYear(day));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadDaily2(TableCellInfo* tci)
{
    // Generates the 2nd header line for daily calendar views.
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
    {
        int dom = dayOfMonth(day);
        s() << "   <td style=\"";
        QColor bgCol = colors.getColor("header");
        if (isWeekend(day))
            bgCol = bgCol.dark(130);
        if (isSameDay(report->getProject()->getNow(), day))
            bgCol = colors.getColor("today");
        s() << "background-color:" << bgCol.name() << "; "
            << "font-size:80%; text-align:center\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
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
        if (dom < 10)
            s() << "&nbsp;";
        generateSubTitle(tci, QString().sprintf("%d", dom));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadWeekly1(TableCellInfo* tci)
{
    // Generates the 1st header line for weekly calendar views.
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end; )
    {
        int currMonth = monthOfWeek(week, weekStartsMonday);
        int left;
        time_t wi = sameTimeNextWeek(week);
        for (left = 1 ; wi < end &&
             monthOfWeek(wi, weekStartsMonday) == currMonth;
             wi = sameTimeNextWeek(wi))
            left++;
             
        s() << "   <td colspan=\""
          << QString().sprintf("%d", left) << "\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
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
                      QString("%1 %2").arg(shortMonthName(monthOfWeek(week, 
                                                          weekStartsMonday) 
                                                          - 1)).
                      arg(yearOfWeek(week, weekStartsMonday)));
        s() << "</td>" << endl;
        week = wi;
    }
}

void
HTMLReportElement::genHeadWeekly2(TableCellInfo* tci)
{
    // Generates the 2nd header line for weekly calendar views.
    bool wsm = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, wsm); week < end; 
         week = sameTimeNextWeek(week))
    {
        int woy = weekOfYear(week, wsm);
        s() << "   <td style=\"";
        QColor bgCol;
        if (isSameWeek(report->getProject()->getNow(), week, wsm))
            bgCol = colors.getColor("today");
        else
            bgCol = colors.getColor("header");
        s() << "background-color:" << bgCol.name() << "; "
            << "font-size:80%; text-align:center\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
        if (woy < 10)
            s() << "&nbsp;";
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
        generateSubTitle(tci, QString().sprintf("%d", woy));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadMonthly1(TableCellInfo* tci)
{
    // Generates 1st header line of monthly calendar view.
    for (time_t year = beginOfMonth(start); year < end;
         year = sameTimeNextYear(beginOfYear(year)))
    {
        int left = monthLeftInYear(year);
        if (left > monthsBetween(year, end))
            left = monthsBetween(year, end);
        s() << "   <td colspan=\""
          << QString().sprintf("%d", left) << "\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
        mt.setMacro(new Macro(KW("day"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"), "1",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", ::year(year)),
                              defFileName, defFileLine));
        generateTitle(tci, QString().sprintf("%d", ::year(year)));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadMonthly2(TableCellInfo* tci)
{
    // Generates 2nd header line of monthly calendar view.
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        int moy = monthOfYear(month);
        s() << "   <td style=\"";
        QColor bgCol;
        if (isSameMonth(report->getProject()->getNow(), month))
            bgCol = colors.getColor("today");
        else
            bgCol = colors.getColor("header");
        s() << "background-color:" << bgCol.name() << "; "
            << "font-size:80%; text-align:center\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
        if (month < 10)
            s() << "&nbsp;";
        mt.setMacro(new Macro(KW("day"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", moy),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"),
                              QString().sprintf
                              ("%02d", quarterOfYear(month)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"),
                              QString().sprintf
                              ("%02d", weekOfYear(month, weekStartsMonday)),
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(month)),
                              defFileName, defFileLine));
        generateSubTitle(tci, shortMonthName(moy - 1));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadQuarterly1(TableCellInfo* tci)
{
    // Generates 1st header line of quarterly calendar view.
    for (time_t year = beginOfQuarter(start); year < end;
         year = sameTimeNextYear(beginOfYear(year)))
    {
        int left = quartersLeftInYear(year);
        if (left > quartersBetween(year, end))
            left = quartersBetween(year, end);
        s() << "   <td colspan=\""
          << QString().sprintf("%d", left) << "\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
        mt.setMacro(new Macro(KW("day"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("month"), "01",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("quarter"), "1",
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("week"), "01", 
                              defFileName, defFileLine));
        mt.setMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", ::year(year)),
                              defFileName, defFileLine));
        generateTitle(tci, QString().sprintf("%d", ::year(year)));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadQuarterly2(TableCellInfo* tci)
{
    // Generates 2nd header line of quarterly calendar view.
    static const char* qnames[] =
    {
        I18N_NOOP("1st Quarter"), I18N_NOOP("2nd Quarter"),
        I18N_NOOP("3rd Quarter"), I18N_NOOP("4th Quarter")
    };

    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        int qoy = quarterOfYear(quarter);
        s() << "   <td style=\"";
        QColor bgCol;
        if (isSameQuarter(report->getProject()->getNow(), quarter))
            bgCol = colors.getColor("today");
        else
            bgCol = colors.getColor("header");
        s() << "background-color:" << bgCol.name() << "; "
            << "font-size:80%; text-align:center\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
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
        generateSubTitle(tci, i18n(qnames[qoy - 1]));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadYear(TableCellInfo* tci)
{
    // Generates 1st header line of monthly calendar view.
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        s() << "   <td rowspan=\"2\"";
        if (((HTMLReport*) report)->hasStyleSheet())
            s() << " class=\"tj_header_cell\"";
        s() << ">";
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
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genCellEmpty(TableCellInfo* tci)
{
    genCell("", tci, TRUE);
}

void
HTMLReportElement::genCellSequenceNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? 
            QString().sprintf("%d.", tci->tli->ca1->getSequenceNo()) :
            QString(""), tci, TRUE);
}

void
HTMLReportElement::genCellNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? QString().sprintf("%d.", tci->tli->idxNo) :
            QString(""), tci, TRUE);
}

void
HTMLReportElement::genCellHierarchNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ?
            tci->tli->ca1->getHierarchNo() : QString(""), tci, TRUE);
}

void
HTMLReportElement::genCellIndex(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? 
            QString().sprintf("%d.", tci->tli->ca1->getIndex()) :
            QString(""), tci, TRUE);
}

void
HTMLReportElement::genCellHierarchIndex(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ?
            tci->tli->ca1->getHierarchIndex() : QString(""), tci, TRUE);
}

void
HTMLReportElement::genCellId(TableCellInfo* tci)
{
    genCell(tci->tli->ca1->getId(), tci, TRUE); 
}

void
HTMLReportElement::genCellName(TableCellInfo* tci)
{
    int lPadding = 0;
    int fontSize = tci->tli->ca2 == 0 ? 100 : 90; 
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
            tci->setFontFactor(fontSize + 5 * (maxDepthTaskList - 1 - 
                                               tci->tli->ca1->treeLevel()));
        }
        tci->setLeftPadding(2 + lPadding * 15);
        text = tci->tli->ca1->getName(); 
    }
    else
        text = tci->tli->specialName;
    genCell(text, tci, TRUE);
}

void
HTMLReportElement::genCellStart(TableCellInfo* tci)
{
    if (!tci->tli->task->isStartOk(tci->tli->sc))
        tci->setBgColor(colors.getColor("error"));
    genCell(time2user(tci->tli->task->getStart(tci->tli->sc), timeFormat),
            tci, FALSE);
}

void
HTMLReportElement::genCellEnd(TableCellInfo* tci)
{
    if (!tci->tli->task->isEndOk(tci->tli->sc))
        tci->setBgColor(colors.getColor("error"));
    genCell(time2user(tci->tli->task->getEnd(tci->tli->sc) +
                      (tci->tli->task->isMilestone() ? 1 : 0), timeFormat),
            tci, FALSE);
}

#define GCMMSE(a) \
void \
HTMLReportElement::genCell##a(TableCellInfo* tci) \
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
HTMLReportElement::genCell##a##Buffer(TableCellInfo* tci) \
{ \
    genCell(QString().sprintf \
            ("%3.0f", tci->tli->task->get##a##Buffer(tci->tli->sc)), \
            tci, FALSE); \
}

GCSEBUFFER(Start)
GCSEBUFFER(End)

void
HTMLReportElement::genCellStartBufferEnd(TableCellInfo* tci)
{
    genCell(time2user(tci->tli->task->getStartBufferEnd
                         (tci->tli->sc), timeFormat), tci, FALSE);
}

void
HTMLReportElement::genCellEndBufferStart(TableCellInfo* tci)
{
    genCell(time2user(tci->tli->task->getStartBufferEnd
                      (tci->tli->sc) + 1, timeFormat), tci, FALSE); 
}

void
HTMLReportElement::genCellDuration(TableCellInfo* tci)
{
    genCell(scaledLoad(tci->tli->task->getCalcDuration(tci->tli->sc), tci),
            tci, FALSE);
}

void
HTMLReportElement::genCellEffort(TableCellInfo* tci)
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
    
    generateRightIndented(tci, scaledLoad(val, tci));
}

void
HTMLReportElement::genCellProjectId(TableCellInfo* tci)
{
    genCell(tci->tli->task->getProjectId() + " (" +
            report->getProject()->getIdIndex(tci->tli->task->
                                             getProjectId()) + ")", tci,
            TRUE);
}

void
HTMLReportElement::genCellResources(TableCellInfo* tci)
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
HTMLReportElement::genCellResponsible(TableCellInfo* tci)
{
    if (tci->tli->task->getResponsible())
        genCell(tci->tli->task->getResponsible()->getName(),
                tci, TRUE);
    else
        genCell("", tci, TRUE);
}

void
HTMLReportElement::genCellText(TableCellInfo* tci)
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
HTMLReportElement::genCellStatusNote(TableCellInfo* tci)
{
    if (tci->tli->task->getStatusNote(tci->tli->sc).isEmpty())
        genCell("", tci, TRUE);
    else
        genCell(tci->tli->task->getStatusNote(tci->tli->sc),
                tci, TRUE);
}

void
HTMLReportElement::genCellCost(TableCellInfo* tci)
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
HTMLReportElement::genCellRevenue(TableCellInfo* tci)
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
HTMLReportElement::genCellProfit(TableCellInfo* tci)
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
HTMLReportElement::genCellPriority(TableCellInfo* tci)
{
    genCell(QString().sprintf("%d", tci->tli->task->getPriority()),
            tci, TRUE);
}

void
HTMLReportElement::genCellFlags(TableCellInfo* tci)
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
HTMLReportElement::genCellCompleted(TableCellInfo* tci)
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
HTMLReportElement::genCellStatus(TableCellInfo* tci)
{
    QString text;
    switch (tci->tli->task->getStatus(tci->tli->sc))
    {
    case NotStarted:
        text = i18n("Not yet started");
        break;
    case InProgressLate:
        text = i18n("Behind schedule");
        break;
    case InProgress:
        text = i18n("Work in progress");
        break;
    case OnTime:
        text = i18n("On schedule");
        break;
    case InProgressEarly:
        text = i18n("Ahead of schedule");
        break;
    case Finished:
        text = i18n("Finished");
        break;
    default:
        text = i18n("Unknown status");
        break;
    }
    genCell(text, tci, FALSE);
}

void
HTMLReportElement::genCellReference(TableCellInfo* tci)
{
    if (tci->tcf->getId() == "reference")
    {
        if (tci->tli->task->getReference().isEmpty())
            genCell("", tci, TRUE);
        else
        {
            QString text ="<a href=\"" + tci->tli->task->getReference() + "\">";
            if (tci->tli->task->getReferenceLabel().isEmpty())
                text += htmlFilter(tci->tli->task->getReference());
            else
                text += htmlFilter(tci->tli->task->getReferenceLabel());
            text += "</a>";
            genCell(text, tci, TRUE, FALSE);
        }
        return;
    }
   
    const ReferenceAttribute* ra =  (const ReferenceAttribute*)
        tci->tli->ca1->getCustomAttribute(tci->tcf->getId());
    if (!ra || ra->getUrl().isEmpty())
        genCell("", tci, TRUE);
    else
    {
        QString text ="<a href=\"" + ra->getUrl() + "\">";
        if (ra->getLabel().isEmpty())
            text += htmlFilter(ra->getUrl());
        else
            text += htmlFilter(ra->getLabel());
        text += "</a>";
        genCell(text, tci, TRUE, FALSE);
    }
}

void
HTMLReportElement::genCellScenario(TableCellInfo* tci)
{
    genCell(report->getProject()->getScenarioName(tci->tli->sc), tci, FALSE);
}

#define GCDEPFOL(a, b) \
void \
HTMLReportElement::genCell##a(TableCellInfo* tci) \
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
HTMLReportElement::selectTaskBgColor(TableCellInfo* tci, double load,
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
HTMLReportElement::selectResourceBgColor(TableCellInfo* tci, double load,
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
HTMLReportElement::genCellTaskFunc(TableCellInfo* tci, bool daily,
                                   time_t (*beginOfT)(time_t),
                                   time_t (*sameTimeNextT)(time_t))
{
    for (time_t t = (*beginOfT)(start); t < end; t = (*sameTimeNextT)(t))
    {
        Interval period = Interval(t, sameTimeNextT(t) - 1);
        double load = tci->tli->task->getLoad(tci->tli->sc, period, 
                                              tci->tli->resource);
        QColor bgCol = selectTaskBgColor(tci, load, period, daily);

        int runLength = 1;
        if (!tci->tli->task->isActive(tci->tli->sc, period))
        {
            time_t lastEndT = t;
            for (time_t endT = sameTimeNextT(t); endT < end;
                 endT = sameTimeNextT(endT))
            {
                Interval periodProbe = Interval(endT, sameTimeNextT(endT) - 1);
                double loadProbe = tci->tli->task->getLoad(tci->tli->sc,
                                                           periodProbe, 
                                                           tci->tli->resource);
                QColor bgColProbe = selectTaskBgColor(tci, loadProbe,
                                                      periodProbe, daily);
                if (load != loadProbe || bgCol != bgColProbe)
                    break;
                lastEndT = endT;
                runLength++;
            }
            t = lastEndT;
        }
        tci->setColumns(runLength);
        tci->setBgColor(bgCol);
        
        reportTaskLoad(load, tci, period);
    }
}

void
HTMLReportElement::genCellResourceFunc(TableCellInfo* tci, bool daily,
                                   time_t (*beginOfT)(time_t),
                                   time_t (*sameTimeNextT)(time_t))
{
    for (time_t t = beginOfT(start); t < end; t = sameTimeNextT(t))
    {
        Interval period = Interval(t, sameTimeNextT(t) - 1);
        double load = tci->tli->resource->getLoad(tci->tli->sc, period,
                                                  AllAccounts, tci->tli->task);
        QColor bgCol = selectResourceBgColor(tci, load, period, daily); 

        int runLength = 1;
        if (load == 0.0)
        {
            time_t lastEndT = t;
            for (time_t endT = sameTimeNextT(t); endT < end;
                 endT = sameTimeNextT(endT))
            {
                Interval periodProbe = Interval(endT, sameTimeNextT(endT) - 1);
                double loadProbe = 
                    tci->tli->resource->getLoad(tci->tli->sc, periodProbe,
                                                AllAccounts, tci->tli->task);
                QColor bgColProbe = selectResourceBgColor(tci, loadProbe,
                                                          periodProbe, daily);

                if (load != loadProbe || bgCol != bgColProbe)
                    break;
                lastEndT = endT;
                runLength++;
            }
            t = lastEndT;
        }
        tci->setColumns(runLength);
        tci->setBgColor(bgCol);
        
        reportResourceLoad(load, tci, period);
    }
}

void
HTMLReportElement::genCellAccountFunc(TableCellInfo* tci,
                                      time_t (*beginOfT)(time_t),
                                      time_t (*sameTimeNextT)(time_t))
{
    tci->tcf->realFormat = currencyFormat;
    for (time_t t = beginOfT(start); t < end; t = sameTimeNextT(t))
    {
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
HTMLReportElement::genCellDailyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, TRUE, midnight, sameTimeNextDay);
}

void
HTMLReportElement::genCellDailyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, TRUE, midnight, sameTimeNextDay);
}

void
HTMLReportElement::genCellDailyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, midnight, sameTimeNextDay);
}

void
HTMLReportElement::genCellWeeklyTask(TableCellInfo* tci)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        Interval period = Interval(week, sameTimeNextWeek(week) - 1);
        double load = tci->tli->task->getLoad(tci->tli->sc, period, 
                                              tci->tli->resource);
        QColor bgCol = selectTaskBgColor(tci, load, period, FALSE);

        int runLength = 1;
        if (!tci->tli->task->isActive(tci->tli->sc, period))
        {
            time_t lastEndWeek = week;
            for (time_t endWeek = sameTimeNextWeek(week); endWeek < end;
                 endWeek = sameTimeNextWeek(endWeek))
            {
                Interval periodProbe = Interval(endWeek)
                    .firstWeek(weekStartsMonday);
                double loadProbe = tci->tli->task->getLoad(tci->tli->sc,
                                                           periodProbe, 
                                                           tci->tli->resource);
                QColor bgColProbe = selectTaskBgColor(tci, loadProbe,
                                                      periodProbe, FALSE);
                if (load != loadProbe || bgCol != bgColProbe)
                    break;
                lastEndWeek = endWeek;
                runLength++;
            }
            week = lastEndWeek;
        }
        tci->setColumns(runLength);
        tci->setBgColor(bgCol);

        reportTaskLoad(load, tci, period);
    }
}

void
HTMLReportElement::genCellWeeklyResource(TableCellInfo* tci)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        Interval period = Interval(week, sameTimeNextWeek(week) - 1);
        double load = tci->tli->resource->getLoad(tci->tli->sc, period,
                                                  AllAccounts, tci->tli->task);
        QColor bgCol = selectResourceBgColor(tci, load, period, FALSE);

        int runLength = 1;
        if (load == 0.0)
        {
            time_t lastEndWeek = week;
            for (time_t endWeek = sameTimeNextWeek(week); endWeek < end;
                 endWeek = sameTimeNextWeek(endWeek))
            {
                Interval periodProbe = Interval(endWeek)
                    .firstWeek(weekStartsMonday);
                double loadProbe = 
                    tci->tli->resource->getLoad(tci->tli->sc, periodProbe,
                                                AllAccounts, tci->tli->task);
                QColor bgColProbe = selectResourceBgColor(tci, loadProbe,
                                                          periodProbe, FALSE);
                if (load != loadProbe || bgCol != bgColProbe)
                    break;
                lastEndWeek = endWeek;
                runLength++;
            }
            week = lastEndWeek;
        }
        tci->setColumns(runLength); 
        tci->setBgColor(bgCol);
        
        reportResourceLoad(load, tci, period);
    }
}

void
HTMLReportElement::genCellWeeklyAccount(TableCellInfo* tci)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        double volume = tci->tli->account->
            getVolume(tci->tli->sc, Interval(week, sameTimeNextWeek(week) - 1));
        if ((accountSortCriteria[0] == CoreAttributesList::TreeMode && 
             tci->tli->account->isRoot()) ||
            (accountSortCriteria[0] != CoreAttributesList::TreeMode)) 
            tci->tci->addToSum(tci->tli->sc, time2ISO(week), volume);
        reportCurrency(volume, tci, week);
    }
}

void
HTMLReportElement::genCellMonthlyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, FALSE, beginOfMonth, sameTimeNextMonth);
}

void
HTMLReportElement::genCellMonthlyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, FALSE, beginOfMonth, sameTimeNextMonth);   
}

void
HTMLReportElement::genCellMonthlyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, beginOfMonth, sameTimeNextMonth);
}

void
HTMLReportElement::genCellQuarterlyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, FALSE, beginOfQuarter, sameTimeNextQuarter);
}

void
HTMLReportElement::genCellQuarterlyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, FALSE, beginOfQuarter, sameTimeNextQuarter);
}

void
HTMLReportElement::genCellQuarterlyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, beginOfQuarter, sameTimeNextQuarter);
}

void
HTMLReportElement::genCellYearlyTask(TableCellInfo* tci)
{
    genCellTaskFunc(tci, FALSE, beginOfYear, sameTimeNextYear);
}

void
HTMLReportElement::genCellYearlyResource(TableCellInfo* tci)
{
    genCellResourceFunc(tci, FALSE, beginOfYear, sameTimeNextYear);
}

void
HTMLReportElement::genCellYearlyAccount(TableCellInfo* tci)
{
    genCellAccountFunc(tci, beginOfYear, sameTimeNextYear);
}

void
HTMLReportElement::genCellResponsibilities(TableCellInfo* tci)
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
HTMLReportElement::genCellSchedule(TableCellInfo* tci)
{
    s() << "   <td>" << endl;

    if (tci->tli->resource)
    {
        BookingList jobs = tci->tli->resource->getJobs(tci->tli->sc);
        jobs.setAutoDelete(TRUE);
        time_t prevTime = 0;
        Interval reportPeriod(start, end);
        s() << "    <table style=\"width:150px; font-size:100%; "
           << "text-align:left\">" << endl
           << "      <tr>" << endl
           << "       <th style=\"width:35%\"></th>" << endl
           << "       <th style=\"width:65%\"></th>" << endl
           << "      </tr>" << endl;
        for (BookingListIterator bli(jobs); *bli != 0; ++bli)
        {
            if ((tci->tli->ca2 == 0 || tci->tli->task == (*bli)->getTask()) && 
                reportPeriod.overlaps(Interval((*bli)->getStart(), 
                                               (*bli)->getEnd())))
            {
                /* If the reporting interval is not more than a day, we
                 * do not print the day since this information is most
                 * likely given by the context of the report. */
                if (!isSameDay(prevTime, (*bli)->getStart()) &&
                    !isSameDay(start, end - 1))
                {
                    s() << "      <tr>" << endl
                        << "       <td colspan=\"2\" style=\"font-size:120%\">"
                        << time2weekday((*bli)->getStart()) << ", "
                        << time2date((*bli)->getStart()) << "</td>" << endl
                        << "      </tr>" 
                        << endl;
                }
                s() << "       <tr>" << endl
                    << "        <td>";
                Interval workPeriod((*bli)->getStart(), (*bli)->getEnd());
                workPeriod.overlap(reportPeriod);
                s() << time2user(workPeriod.getStart(), shortTimeFormat)
                    << "&nbsp;-&nbsp;"
                    << time2user(workPeriod.getEnd() + 1, shortTimeFormat);
                s() << "</td>" << endl
                    << "       <td>";
                if (tci->tli->ca2 == 0)
                    s() << " " << htmlFilter((*bli)->getTask()->getName());
                s() << "       </td>" << endl;
                prevTime = (*bli)->getStart();
                s() << "      </tr>" << endl;
            }
        }
        s() << "     </table>" << endl;
    }
    else
        s() << "&nbsp;";

    s() << "   </td>" << endl;
}

#define GCEFFORT(a) \
void \
HTMLReportElement::genCell##a##Effort(TableCellInfo* tci) \
{ \
    genCell(tci->tcf->realFormat.format \
            (tci->tli->resource->get##a##Effort(), FALSE), \
            tci, TRUE); \
}

GCEFFORT(Min)
GCEFFORT(Max)

void
HTMLReportElement::genCellRate(TableCellInfo* tci)
{
    genCell(tci->tcf->realFormat.format(tci->tli->resource->getRate(), tci),
            tci, TRUE);
}

void
HTMLReportElement::genCellKotrusId(TableCellInfo* tci)
{
    genCell(tci->tli->resource->getKotrusId(), tci, TRUE);
}

void
HTMLReportElement::genCellTotal(TableCellInfo* tci)
{
    double value = tci->tli->account->getVolume(tci->tli->sc, 
                                                Interval(start, end));
    if (tci->tli->account->isLeaf())
        tci->tci->addToSum(tci->tli->sc, "total", value);
    genCell(tci->tcf->realFormat.format(value, tci), tci, FALSE);
}

void
HTMLReportElement::genCellSummary(TableCellInfo* tci)
{
    QMap<QString, double>::ConstIterator it;
    const QMap<QString, double>* sum = tci->tci->getSum();
    if (sum)
    {
        uint sc = tci->tli->sc;
        double val = 0.0;
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
        genCell("", tci, FALSE);
}

