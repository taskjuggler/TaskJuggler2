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
#include "TableColumn.h"
#include "TableColumnFormat.h"
#include "TableLineInfo.h"

#define KW(a) a

HTMLReportElement::HTMLReportElement(Report* r, const QString& df, int dl) :
   ReportElement(r, df, dl)
{
    barLabels = BLT_LOAD;

    registerUrl(KW("dayheader"));
    registerUrl(KW("monthheader"));
    registerUrl(KW("resourcename"));
    registerUrl(KW("taskname"));
    registerUrl(KW("weekheader"));
    registerUrl(KW("yearheader"));
}

void
HTMLReportElement::generateFirstTask(const Task* t, const Resource* r, uint no)
{
    s() << "  <tr valign=\"middle\">" << endl;
   
    TableLineInfo tli(t, r, t, r, 0, no, 0);

    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genTaskLine1)
        {
            (*this.*(columnFormat[(*it)->getName()]->genTaskLine1))
                (&tli);
        }
        else
            qFatal("generateFirstTask: Unknown Column %s",
                   ((*it)->getName()).latin1());
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateNextTask(int sc, const Task* t, const Resource* r)
{
    TableLineInfo tli(t, r, t, r, 0, 0, sc);

    s() << "  <tr>" << endl;
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genTaskLine2)
        {
            (*this.*(columnFormat[(*it)->getName()]->genTaskLine2))
                (&tli);
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateFirstResource(const Resource* r, const Task* t, 
                                        uint no)
{
    TableLineInfo tli(r, t, t, r, 0, no, 0);
    
    s() << "  <tr valign=\"middle\">";
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genResourceLine1)
        {
            (*this.*(columnFormat[(*it)->getName()]->genResourceLine1))
                (&tli);
        }
        else
            qFatal("generateFirstResource: Unknown Column %s",
                   ((*it)->getName()).latin1());
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateNextResource(int sc, const Resource* r, 
                                        const Task* t)
{
    TableLineInfo tli(r, t, t, r, 0, 0, sc);
    
    s() << "  <tr valign=\"middle\">";
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genResourceLine2)
        {
            (*this.*(columnFormat[(*it)->getName()]->genResourceLine2))
                (&tli);
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateFirstAccount(const Account* a, uint no)
{
    TableLineInfo tli(a, 0, 0, 0, a, no, 0);

    s() << "  <tr valign=\"middle\">" << endl;
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genAccountLine1)
        {
            (*this.*(columnFormat[(*it)->getName()]->genAccountLine1))
                (&tli);
        }
        else
            qFatal("generateFirstAccount: Unknown Column %s",
                   ((*it)->getName()).latin1());
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateNextAccount(int sc, const Account* a)
{
    TableLineInfo tli(a, 0, 0, 0, a, 0, sc);
    
    s() << "  <tr valign=\"middle\">";
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genAccountLine2)
        {
            (*this.*(columnFormat[(*it)->getName()]->genAccountLine2))
                (&tli);
        }
    }
    s() << "  </tr>" << endl;
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
    s() << "<table align=\"center\" cellpadding=\"2\">" << endl;
    s() << " <thead>" << endl 
        << "  <tr valign=\"middle\">" << endl;
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()])
            (*this.*(columnFormat[(*it)->getName()]->genHeadLine1))
                (columnFormat[(*it)->getName()]);
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
    bool td = FALSE;
    s() << "  <tr>" << endl;
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()])
            if (columnFormat[(*it)->getName()]->genHeadLine2)
            {
                (*this.*(columnFormat[(*it)->getName()]->genHeadLine2))
                    (columnFormat[(*it)->getName()]);
                td = TRUE;
            }
    }
    if (!td)
        s() << "   <td>&nbsp;</td>" << endl;
    s() << "  </tr>" << endl << " </thead>\n" << endl;
}

void
HTMLReportElement::generateSummary(const QString& name, const QString&)
{
    TableLineInfo tli(0, 0, 0, 0, 0, 0, 0);
    
    s() << "  <tr valign=\"middle\">" << endl;
    for (QPtrListIterator<TableColumn> it(columns); it; ++it )
    {
        if ((*it)->getName() == "name")
            singleRowCell(name, &tli); 
        else if (columnFormat[(*it)->getName()])
        {
            if (columnFormat[(*it)->getName()]->genSummaryLine)
            {
                (*this.*(columnFormat[(*it)->getName()]->genSummaryLine))
                    (&tli);
            }
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::singleRowCell(const QString& text, TableLineInfo*)
{
    s() << "   <td ";
    s() << ">" << text << "</td>" << endl;
}

void
HTMLReportElement::multiRowCell(const QString& text, TableLineInfo*)
{
    s() << "   <td class=\""
        << "\" rowspan=\""
        << QString("%1").arg(scenarios.count()) << "\"";
    s() << ">" << text << "</td>" << endl;
}

void
HTMLReportElement::textOneRow(const QString& text, bool light, 
                              const QString& align)
{
    s() << "   <td class=\""
      << (light ? "defaultlight" : "default") << "\"";
    if (!align.isEmpty())
        s() << " style=\"text-align:" << align << "; white-space:nowrap\"";
    s() << ">" << text << "</td>" << endl;
}

void
HTMLReportElement::textMultiRows(const QString& text, bool light, 
                                 const QString& align)
{
    s() << "   <td class=\""
        << (light ? "defaultlight" : "default")
        << "\" rowspan=\""
        << QString("%1").arg(scenarios.count()) << "\"";
    if (!align.isEmpty())
        s() << " style=\"text-align:" << align << "; white-space:nowrap\"";
    s() << ">" << text << "</td>" << endl;
}

void
HTMLReportElement::reportLoad(double load, const QString& bgCol, bool bold,
                       bool milestone)
{
    if ((load > 0.0 || milestone) && barLabels != BLT_EMPTY)
    {
        s() << "   <td class=\""
          << bgCol << "\">";
        if (bold)
            s() << "<b>";
        if (milestone)
            s() << "*";
        else
            s() << scaledLoad(load);
        if (bold)
            s() << "</b>";
        s() << "</td>" << endl;
    }
    else
        s() << "   <td class=\""
          << bgCol << "\">&nbsp;</td>" << endl;
}


void
HTMLReportElement::reportValue(double value, const QString& bgCol, bool bold)
{
    s() << "   <td class=\""
      << bgCol << "\" style=\"text-align:right\">";
    if (bold)
        s() << "<b>";
    s() << QString().sprintf("%.*f", 
                           report->getProject()->getCurrencyDigits(), value);
//      scaleTime(value, FALSE);
    if (bold)
        s() << "</b>";
    s() << "</td>" << endl;
}

void
HTMLReportElement::reportPIDs(const QString& pids, const QString bgCol, 
                              bool bold)
{
    s() << "   <td class=\""
      << bgCol << "\" style=\"white-space:nowrap\">";
    if (bold)
        s() << "<b>";
    s() << pids;
    if (bold)
        s() << "</b>";
    s() << "</td>" << endl;
}

QString
HTMLReportElement::generateUrl(const QString& key, const QString& txt)
{
    if (getUrl(key))
    {
        mt.setLocation(defFileName, defFileLine);
        return QString("<a href=\"") + mt.expand(*getUrl(key))
            + "\">" + htmlFilter(txt) + "</a>";
    }
    else
        return htmlFilter(txt);
}

void
HTMLReportElement::generateRightIndented(TableLineInfo* tli,
                                         const QString str)
{
    int topIndent = 0, subIndent = 0;
    if (strcmp(tli->ca1->getType(), "Task") == 0)
    {
        if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
            subIndent = tli->ca2 == 0 ? 8 : 5;
        if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
            topIndent = (tli->ca2 != 0 ? 0 : 5) * maxDepthResourceList;
    }
    else if (strcmp(tli->ca1->getType(), "Resource") == 0)
    {
        if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
            subIndent = tli->ca2 == 0 ? 8 : 5;
        if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
            topIndent = (tli->ca2 != 0 ? 0 : 5) * maxDepthTaskList;
    }
    
    s() << "   <td class=\"default" << (tli->ca2 == 0 ? "" : "light")
        << "\" style=\"text-align:right; white-space:nowrap;"
        << " padding-right:" 
        << QString("%1").arg(2 + topIndent +
                             (maxDepthTaskList - 1 - tli->ca1->treeLevel()) * 
                             subIndent)
        << "\">"
        << str
        << "</td>" << endl;
}

void
HTMLReportElement::genHeadDefault(TableColumnFormat* tcf)
{
    s() << "   <td class=\"headerbig\" rowspan=\"2\">"
        << htmlFilter(tcf->getTitle()) << "</td>" << endl;
}

void
HTMLReportElement::genHeadCurrency(TableColumnFormat* tcf)
{
    s() << "   <td class=\"headerbig\" rowspan=\"2\">"
        << htmlFilter(i18n(tcf->getTitle()));
    if (!report->getProject()->getCurrency().isEmpty())
        s() << " " << htmlFilter(report->getProject()->getCurrency());
    s() << "</td>" << endl;
}

void
HTMLReportElement::genHeadDaily1(TableColumnFormat*)
{
    // Generates the 1st header line for daily calendar views.
    for (time_t day = midnight(start); day < end;
         day = beginOfMonth(sameTimeNextMonth(day)))
    {
        int left = daysLeftInMonth(day);
        if (left > daysBetween(day, end))
            left = daysBetween(day, end);
        s() << "   <td class=\"headerbig\" colspan=\""
            << QString().sprintf("%d", left) << "\">"; 
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(day)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(day)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(day)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("monthheader"), monthAndYear(day)); 
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadDaily2(TableColumnFormat*)
{
    // Generates the 2nd header line for daily calendar views.
    for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
    {
        int dom = dayOfMonth(day);
        s() << "   <td class=\"" <<
            (isSameDay(report->getProject()->getNow(), day) ?
             "today" : isWeekend(day) ? "weekend" : "headersmall")
          << "\"><span style=\"font-size:0.8em\">&nbsp;";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d", dom),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(day)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(day)),
                              defFileName, defFileLine));
        if (dom < 10)
            s() << "&nbsp;";
        s() << generateUrl(KW("dayheader"), QString().sprintf("%d", dom));
        s() << "</span></td>" << endl;
    }
}

void
HTMLReportElement::genHeadWeekly1(TableColumnFormat*)
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
             
        s() << "   <td class=\"headerbig\" colspan=\""
          << QString().sprintf("%d", left) << "\">";
        mt.clear();
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfWeek(week,
                                                            weekStartsMonday)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", yearOfWeek(week,
                                                            weekStartsMonday)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("monthheader"), 
                         QString("%1 %2").
                         arg(shortMonthName(monthOfWeek(week, weekStartsMonday)
                                            - 1)).
                         arg(yearOfWeek(week, weekStartsMonday)));
        s() << "</td>" << endl;
        week = wi;
    }
}

void
HTMLReportElement::genHeadWeekly2(TableColumnFormat*)
{
    // Generates the 2nd header line for weekly calendar views.
    bool wsm = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, wsm); week < end; 
         week = sameTimeNextWeek(week))
    {
        int woy = weekOfYear(week, wsm);
        s() << "   <td class=\"" <<
            (isSameWeek(report->getProject()->getNow(), week, wsm) ?
             "today" : "headersmall")
          << "\"><span style=\"font-size:0.8em\">&nbsp;";
        if (woy < 10)
            s() << "&nbsp;";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(woy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(woy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(woy)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("weekheader"), QString().sprintf("%d", woy));
        s() << "</span></td>" << endl;
    }
}

void
HTMLReportElement::genHeadMonthly1(TableColumnFormat*)
{
    // Generates 1st header line of monthly calendar view.
    for (time_t year = midnight(start); year < end;
         year = beginOfYear(sameTimeNextYear(year)))
    {
        int left = monthLeftInYear(year);
        if (left > monthsBetween(year, end))
            left = monthsBetween(year, end);
        s() << "   <td class=\"headerbig\" colspan=\""
          << QString().sprintf("%d", left) << "\">";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", ::year(year)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("yearheader"),
                         QString().sprintf("%d", ::year(year)));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadMonthly2(TableColumnFormat*)
{
    // Generates 2nd header line of monthly calendar view.
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        int moy = monthOfYear(month);
        s() << "   <td class=\"" <<
            (isSameMonth(report->getProject()->getNow(), month) ?
             "today" : "headersmall")
          << "\"><span style=\"font-size:0.8em\">&nbsp;";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(moy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(moy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(moy)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("monthheader"), shortMonthName(moy - 1));
        s() << "</span></td>" << endl;
    }
}

void
HTMLReportElement::genHeadQuarterly1(TableColumnFormat*)
{
    // Generates 1st header line of quarterly calendar view.
    for (time_t year = midnight(start); year < end;
         year = beginOfYear(sameTimeNextYear(year)))
    {
        int left = quartersLeftInYear(year);
        if (left > quartersBetween(year, end))
            left = quartersBetween(year, end);
        s() << "   <td class=\"headerbig\" colspan=\""
          << QString().sprintf("%d", left) << "\">";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("quarter"),
                              QString().sprintf("%d", quarterOfYear(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", ::year(year)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("yearheader"),
                         QString().sprintf("%d", ::year(year)));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genHeadQuarterly2(TableColumnFormat*)
{
    // Generates 2nd header line of quarterly calendar view.
    static const char* qnames[] =
    {
        I18N_NOOP("1st Quarter"), I18N_NOOP("2nd Quarter"),
        I18N_NOOP("3rd Quarter"), I18N_NOOP("4th Quarter")
    };

    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        int qoy = quarterOfYear(quarter);
        s() << "   <td class=\"" <<
            (isSameQuarter(report->getProject()->getNow(), quarter) ?
             "today" : "headersmall")
          << "\"><span style=\"font-size:0.8em\">&nbsp;";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(qoy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(qoy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("quarter"),
                              QString().sprintf("%d", quarterOfYear(qoy)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", year(qoy)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("quarterheader"), i18n(qnames[qoy - 1]));
        s() << "</span></td>" << endl;
    }
}

void
HTMLReportElement::genHeadYear(TableColumnFormat*)
{
    // Generates 1st header line of monthly calendar view.
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        s() << "   <td class=\"headerbig\" rowspan=\"2\">";
        mt.clear();
        mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
                                                           dayOfMonth(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("month"),
                              QString().sprintf("%02d", monthOfYear(year)),
                              defFileName, defFileLine));
        mt.addMacro(new Macro(KW("year"),
                              QString().sprintf("%04d", ::year(year)),
                              defFileName, defFileLine));
        s() << generateUrl(KW("yearheader"),
                         QString().sprintf("%d", ::year(year)));
        s() << "</td>" << endl;
    }
}

void
HTMLReportElement::genCellEmpty(TableLineInfo* tli)
{
    s() << "   <td class=\""
      << (tli->ca2 != 0 ? "defaultlight" : "default")
      << "\" rowspan=\""
      << QString("%1").arg(scenarios.count())
      << "\">&nbsp;</td>" << endl;
}

void
HTMLReportElement::genCellSequenceNo(TableLineInfo* tli)
{
    textMultiRows((tli->ca2 == 0 ? 
                   QString().sprintf("%d.", tli->ca1->getSequenceNo()) :
                   QString("&nbsp;")), tli->ca2 != 0, "");
}

void
HTMLReportElement::genCellNo(TableLineInfo* tli)
{
    textMultiRows((tli->ca2 == 0 ? QString().sprintf("%d.", tli->no) :
                   QString("&nbsp;")), tli->ca2 != 0, "");
}

void
HTMLReportElement::genCellIndex(TableLineInfo* tli)
{
    textMultiRows((tli->ca2 == 0 ? QString().sprintf("%d.", 
                                                     tli->ca1->getIndex()) :
                   QString("&nbsp;")), tli->ca2 != 0, "");
}

void
HTMLReportElement::genCellId(TableLineInfo* tli)
{
    textMultiRows(htmlFilter(tli->ca1->getId()), tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellName(TableLineInfo* tli)
{
    int lPadding = 0;
    int fontSize = tli->ca2 == 0 ? 100 : 90; 
    if ((tli->ca2 && (strcmp(tli->ca2->getType(), "Resource") == 0 &&
          resourceSortCriteria[0] == CoreAttributesList::TreeMode)) ||
        (tli->ca2 && strcmp(tli->ca2->getType(), "Task") == 0 &&
         taskSortCriteria[0] == CoreAttributesList::TreeMode))
        for (const CoreAttributes* cp = tli->ca2 ; cp != 0;
             cp = cp->getParent())
            lPadding++;

    mt.clear();
    if (tli->task)
        mt.addMacro(new Macro(KW("taskid"), tli->task->getId(), defFileName,
                              defFileLine));
    if (tli->resource)
        mt.addMacro(new Macro(KW("resourceid"), tli->resource->getId(),
                              defFileName, defFileLine));
    if (tli->account)
        mt.addMacro(new Macro(KW("accountid"), tli->account->getId(),
                              defFileName, defFileLine));

    if ((strcmp(tli->ca1->getType(), "Resource") == 0 &&
         resourceSortCriteria[0] == CoreAttributesList::TreeMode) ||
        (strcmp(tli->ca1->getType(), "Task") == 0 &&
         taskSortCriteria[0] == CoreAttributesList::TreeMode) ||
        (strcmp(tli->ca1->getType(), "Account") == 0 &&
         accountSortCriteria[0] == CoreAttributesList::TreeMode))
    {
        lPadding += tli->ca1->treeLevel();
        fontSize = fontSize + 5 * (maxDepthTaskList - 1 - 
                                   tli->ca1->treeLevel()); 
        s() << "   <td class=\""
          << (tli->ca2 == 0 ? "task" : "tasklight") << "\" rowspan=\""
          << QString("%1").arg(scenarios.count())
          << "\" style=\"white-space:nowrap;"
          << " padding-left: " 
          << QString("%1").arg(2 + lPadding * 15) << ";\">"
          << "<span style=\"font-size:" 
          << QString().sprintf("%d", fontSize) << "%\">";
    }
    else
    {
        s() << "   <td class=\""
          << (tli->ca2 == 0 ? "task" : "tasklight") << "\" rowspan=\""
          << QString("%1").arg(scenarios.count())
          << "\" style=\"white-space:nowrap;"
          << " padding-left: " 
          << QString("%1").arg(2 + lPadding * 15)
          << ";\"><span>";
    }
    if (strcmp(tli->ca1->getType(), "Task") == 0)
    {
        if (tli->ca2 == 0)
            s() << "<a name=\"task_" << tli->ca1->getId() << "\"></a>";
        s() << generateUrl(KW("taskname"), tli->ca1->getName());
    }
    else if (strcmp(tli->ca1->getType(), "Resource") == 0)
    {
        if (tli->ca2 == 0)
            s() << "<a name=\"resource_" << tli->ca1->getFullId() 
                << "\"></a>";
        s() << generateUrl(KW("resourcename"), tli->ca1->getName());
    }
    else if (strcmp(tli->ca1->getType(), "Account") == 0)
    {
        if (tli->ca2 == 0)
            s() << "<a name=\"account_" << tli->ca1->getId() << "\"></a>";
        s() << generateUrl(KW("accountname"), tli->ca1->getName());
    }
    s() << "</span></td>" << endl;
}

#define GCSE(a) \
void \
HTMLReportElement::genCell##a(TableLineInfo* tli) \
{ \
    s() << "   <td class=\"" \
        << (tli->task->is##a##Ok(tli->sc) ? \
            (tli->resource == 0 ? "default" : "defaultlight") : "milestone") \
        << "\" style=\"text-align:left white-space:nowrap\">" \
        << time2user(tli->task->get##a(tli->sc), timeFormat) \
        << "</td>" << endl; \
}

GCSE(Start)
GCSE(End)
    
#define GCMMSE(a) \
void \
HTMLReportElement::genCell##a(TableLineInfo* tli) \
{ \
    textMultiRows(tli->task->get##a() == 0 ? "&nbsp;" : \
                  time2user(tli->task->get##a(), timeFormat), \
                  tli->resource != 0, "&nbsp;"); \
}

GCMMSE(MinStart)
GCMMSE(MaxStart)
GCMMSE(MinEnd)
GCMMSE(MaxEnd)

#define GCSEBUFFER(a) \
void \
HTMLReportElement::genCell##a##Buffer(TableLineInfo* tli) \
{ \
    textOneRow(QString().sprintf \
               ("%3.0f", tli->task->get##a##Buffer(tli->sc)), \
               tli->resource != 0, "right"); \
}

GCSEBUFFER(Start)
GCSEBUFFER(End)

void
HTMLReportElement::genCellStartBufferEnd(TableLineInfo* tli)
{
    textOneRow(time2user(tli->task->getStartBufferEnd
                         (tli->sc), timeFormat), tli->resource != 0, "left");
}

void
HTMLReportElement::genCellEndBufferStart(TableLineInfo* tli)
{
    textOneRow(time2user(tli->task->getStartBufferEnd
                         (tli->sc) + 1, timeFormat), tli->resource != 0, "left");
}

void
HTMLReportElement::genCellDuration(TableLineInfo* tli)
{
    s() << "   <td class=\""
        << (tli->resource == 0 ? "default" : "defaultlight")
        << "\" style=\"text-align:right white-space:nowrap\">"
        << scaledLoad(tli->task->getCalcDuration(tli->sc))
        << "</td>" << endl;
}

void
HTMLReportElement::genCellEffort(TableLineInfo* tli)
{
    double val = 0.0;
    if (strcmp(tli->ca1->getType(), "Task") == 0)
    {
        val = tli->task->getLoad(tli->sc, Interval(start, end),
                                 tli->resource);
    }
    else if (strcmp(tli->ca1->getType(), "Resource") == 0)
    {
        val = tli->resource->getLoad(tli->sc, Interval(start, end), 
                                     AllAccounts, tli->task);
    }
    
    generateRightIndented(tli, scaledLoad(val));
}

void
HTMLReportElement::genCellProjectId(TableLineInfo* tli)
{
    textMultiRows(tli->task->getProjectId() + " (" +
                  report->getProject()->getIdIndex(tli->task->getProjectId()) + ")",
                  tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellResources(TableLineInfo* tli)
{
    s() << "   <td class=\""
      << (tli->ca2 != 0 ? "defaultlight" : "default")
      << "\" style=\"text-align:left\">"
      << "<span style=\"font-size:100%\">";
    bool first = TRUE;
    for (ResourceListIterator rli(tli->task->getBookedResourcesIterator(tli->sc));
        *rli != 0; ++rli)
    {
        if (!first)
            s() << ", ";
                    
        s() << htmlFilter((*rli)->getName());
        first = FALSE;
    }
    s() << "</span></td>" << endl;
}

void
HTMLReportElement::genCellResponsible(TableLineInfo* tli)
{
    if (tli->task->getResponsible())
        textMultiRows(htmlFilter(tli->task->getResponsible()->getName()),
                      tli->ca2 != 0, "left");
    else
        textMultiRows("&nbsp", tli->ca2 != 0, "left");
}

#define GCNOTE(a, b) \
void \
HTMLReportElement::genCell##a##Note(TableLineInfo* tli) \
{ \
    s() << "   <td class=\"" \
        << (tli->ca2 == 0 ? "default" : "defaultlight") \
        << "\" rowspan=\"" \
        << QString("%1").arg(scenarios.count()) \
        << "\" style=\"text-align:left\">" \
        << "<span style=\"font-size:100%\">"; \
    if (tli->task->get##a##Note(b).isEmpty()) \
        s() << "&nbsp;"; \
    else \
        s() << htmlFilter(tli->task->get##a##Note(b)); \
    s() << "</span></td>" << endl; \
}

GCNOTE(, )
GCNOTE(Status, tli->sc)

void
HTMLReportElement::genCellCost(TableLineInfo* tli)
{
    double val = 0.0;
    if (strcmp(tli->ca1->getType(), "Task") == 0)
    {
        val = tli->task->getCredits(tli->sc, Interval(start, end),
                                       Cost, tli->resource);
    }
    else if (strcmp(tli->ca1->getType(), "Resource") == 0)
    {
        val = tli->resource->getCredits(tli->sc, Interval(start, end),
                                        Cost, tli->task);
    }
    generateRightIndented
        (tli, QString().sprintf("%.*f", report->getProject()->
                                getCurrencyDigits(), val)); 
}

void
HTMLReportElement::genCellRevenue(TableLineInfo* tli)
{
    double val = 0.0;
    if (strcmp(tli->ca1->getType(), "Task") == 0)
    {
        val = tli->task->getCredits(tli->sc, Interval(start, end),
                                       Revenue, tli->resource);
    }
    else if (strcmp(tli->ca1->getType(), "Resource") == 0)
    {
        val = tli->resource->getCredits(tli->sc, Interval(start, end),
                                        Revenue, tli->task);
    }
    generateRightIndented
        (tli, QString().sprintf("%.*f", report->getProject()->getCurrencyDigits(), val));
}

void
HTMLReportElement::genCellProfit(TableLineInfo* tli)
{
    double val = 0.0;
    if (strcmp(tli->ca1->getType(), "Task") == 0)
    {
        val = tli->task->getCredits(tli->sc, Interval(start, end),
                                    Revenue, tli->resource) - 
            tli->task->getCredits(tli->sc, Interval(start, end),
                                  Cost, tli->resource);
    }
    else if (strcmp(tli->ca1->getType(), "Resource") == 0)
    {
        val = tli->resource->getCredits(tli->sc, Interval(start, end),
                                        Revenue, tli->task) -
            tli->resource->getCredits(tli->sc, Interval(start, end),
                                      Cost, tli->task);
    }
    generateRightIndented(tli, QString().sprintf("%.*f", report->getProject()->
                                                 getCurrencyDigits(), val)); 
}

void
HTMLReportElement::genCellPriority(TableLineInfo* tli)
{
    textMultiRows(QString().sprintf("%d", tli->task->getPriority()),
                  tli->ca2 != 0, "right");
}

void
HTMLReportElement::genCellFlags(TableLineInfo* tli)
{
    FlagList allFlags = tli->ca1->getFlagList();
    QString flagStr;
    for (QStringList::Iterator it = allFlags.begin();
         it != allFlags.end(); ++it)
    {
        if (it != allFlags.begin())
            flagStr += ", ";
        flagStr += htmlFilter(*it);
    }
    if (flagStr.isEmpty())
        flagStr = "&nbsp";
    textMultiRows(flagStr, tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellCompleted(TableLineInfo* tli)
{
    if (tli->task->getCompletionDegree(tli->sc) ==
        tli->task->getCalcedCompletionDegree(tli->sc))
    {
        textOneRow(QString("%1%").arg((int) tli->task->getCompletionDegree(tli->sc)),
                   tli->ca2 != 0, "right");
    }
    else
    {
        textOneRow
            (QString("%1% (%2%)")
             .arg((int) tli->task->getCompletionDegree(tli->sc))
             .arg((int) tli->task->getCalcedCompletionDegree(tli->sc)),
             tli->ca2 != 0, "right");
    }
}

void
HTMLReportElement::genCellStatus(TableLineInfo* tli)
{
    QString text;
    switch (tli->task->getStatus(tli->sc))
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
    textOneRow(text, tli->ca2 != 0, "center");
}

void
HTMLReportElement::genCellReference(TableLineInfo* tli)
{
    s() << "   <td class=\""
        << (tli->ca2 == 0 ? "default" : "defaultlight")
        << "\" rowspan=\""
        << QString("%1").arg(scenarios.count())
        << "\" style=\"text-align:left\">"
        << "<span style=\"font-size:100%\">";
    if (tli->task->getReference().isEmpty())
        s() << "&nbsp;";
    else
    {
        s() << "<a href=\"" << tli->task->getReference() << "\">";
        if (tli->task->getReferenceLabel().isEmpty())
            s() << htmlFilter(tli->task->getReference());
        else
            s() << htmlFilter(tli->task->getReferenceLabel());
        s() << "<a>";
    }
    s() << "</span></td>" << endl;
}

void
HTMLReportElement::genCellScenario(TableLineInfo* tli)
{
    textOneRow(htmlFilter(report->getProject()->getScenarioName(tli->sc)),
               tli->ca2 != 0, "left");
}

#define GCDEPFOL(a, b) \
void \
HTMLReportElement::genCell##a(TableLineInfo* tli) \
{ \
    s() << "   <td class=\"" \
      << (tli->ca2 != 0 ? "defaultlight" : "default") \
      << "\" rowspan=\"" \
      << QString("%1").arg(scenarios.count()) \
      << "\" style=\"text-align:left\"><span style=\"font-size:100%\">"; \
    bool first = TRUE; \
    for (TaskListIterator it(tli->task->get##b##Iterator()); *it != 0; ++it) \
    { \
        if (!first) \
            s() << ", "; \
        else \
            first = FALSE; \
        s() << (*it)->getId(); \
    } \
    if (first) \
        s() << "&nbsp;"; \
    s() << "</span</td>" << endl; \
}

GCDEPFOL(Depends, Previous)
GCDEPFOL(Follows, Followers)

void
HTMLReportElement::genCellDailyTask(TableLineInfo* tli)
{
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextDay(day))
    {
        double load = tli->task->getLoad(tli->sc, Interval(day).firstDay(),
                                         tli->resource);
        QString bgCol = 
            tli->task->isActive(tli->sc, Interval(day).firstDay()) ? 
            (tli->task->isCompleted(tli->sc,
                                    sameTimeNextDay(day) - 1) ?
             (tli->ca2 == 0 ? "completed" : "completedlight") :
             tli->task->isMilestone() ? "milestone" :
             (tli->ca2 == 0 && !tli->task->isBuffer(tli->sc, 
                                                    Interval(day).firstDay())
              ? "booked" : "bookedlight")) :
            isSameDay(report->getProject()->getNow(), day) ? "today" :
            isWeekend(day) ? "weekend" :
            report->getProject()->isVacation(day) ? "vacation" :
            (tli->ca2 == 0 ? "default" : "defaultlight");
        if (showPIDs)
        {
            QString pids = tli->resource->
                getProjectIDs(tli->sc, Interval(day).firstDay(), tli->task);
            reportPIDs(pids, bgCol, !tli->resource->isGroup());
        }
        else
            reportLoad(load, bgCol, !tli->task->isContainer(),
                       tli->task->isActive(tli->sc, Interval(day).firstDay()) &&
                       tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellDailyResource(TableLineInfo* tli)
{
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextDay(day))
    {
        double load = tli->resource->getLoad(tli->sc, Interval(day).firstDay(),
                                             AllAccounts, tli->task);
        QString bgCol = 
            load > tli->resource->getMinEffort() * 
            tli->resource->getEfficiency() ?
            (tli->ca2 == 0 ? "booked" :
             (tli->task->isCompleted(tli->sc, 
                                     sameTimeNextDay(day) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameDay(report->getProject()->getNow(), day) ? "today" :
            isWeekend(day) ? "weekend" :
            report->getProject()->isVacation(day) || 
            tli->resource->hasVacationDay(day) ?
            "vacation" :
            (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellDailyAccount(TableLineInfo* tli)
{
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextDay(day))
    {
        QString style("default");
        if (tli->account)
        {
            double volume = 
                tli->account->getVolume(tli->sc, Interval(day).firstDay());
            columnTotals[tli->sc][time2ISO(day)] += volume;
            reportValue(volume, style, FALSE);
        }
        else
            reportValue(columnTotals[tli->sc][time2ISO(day)], style, TRUE);
    }
}

void
HTMLReportElement::genCellWeeklyTask(TableLineInfo* tli)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        double load = tli->task->getLoad(tli->sc, Interval(week).
                                         firstWeek(weekStartsMonday),
                                         tli->resource);
        QString bgCol = 
            tli->task->isActive(tli->sc, 
                                Interval(week).firstWeek(weekStartsMonday)) ?
            (tli->task->isCompleted(tli->sc, sameTimeNextWeek(week) - 1) ?
             (tli->ca2 == 0 ? "completed" : "completedlight") :
             tli->task->isMilestone() ? "milestone" :
             (tli->ca2 == 0 && 
              !tli->task->isBuffer(tli->sc, Interval(week).
                                   firstWeek(weekStartsMonday))
              ? "booked" : "bookedlight")) :
            isSameWeek(report->getProject()->getNow(), week, 
                       weekStartsMonday) ?
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->task->isContainer(), 
                   tli->task->isActive(tli->sc, 
                                       Interval(week).
                                       firstWeek(weekStartsMonday)) && 
                   tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellWeeklyResource(TableLineInfo* tli)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        double load = 
            tli->resource->getLoad(tli->sc, 
                                   Interval(week).firstWeek(weekStartsMonday),
                                   AllAccounts, tli->task);
        QString bgCol =
            load > tli->resource->getMinEffort() * 
            tli->resource->getEfficiency() ?
            (tli->ca2 == 0 ? "booked" :
             (tli->task->isCompleted(tli->sc, sameTimeNextWeek(week) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameWeek(report->getProject()->getNow(), week, 
                       weekStartsMonday) ?
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        if (showPIDs)
        {
            QString pids = tli->resource->
                getProjectIDs(tli->sc, 
                              Interval(week).firstWeek(weekStartsMonday), 
                              tli->task);
            reportPIDs(pids, bgCol, !tli->resource->isGroup());
        }
        else
            reportLoad(load, bgCol, !tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellWeeklyAccount(TableLineInfo*)
{
}

void
HTMLReportElement::genCellMonthlyTask(TableLineInfo* tli)
{
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        double load = tli->task->getLoad(tli->sc, Interval(month).firstMonth(),
                                         tli->resource);
        QString bgCol = 
            tli->task->isActive(tli->sc, 
                                Interval(month).firstMonth()) ?
            (tli->task->isCompleted(tli->sc, 
                                    sameTimeNextMonth(month) - 1) ?
             (tli->ca2 == 0 ? "completed" : "completedlight"):
             tli->task->isMilestone() ? "milestone" :
             (tli->ca2 == 0 &&
              !tli->task->isBuffer(tli->sc, 
                                   Interval(month).firstMonth())
              ? "booked" : "bookedlight")) :
            isSameMonth(report->getProject()->getNow(), month) ? 
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->task->isContainer(), 
                   tli->task->isActive(tli->sc, Interval(month).firstMonth()) && 
                   tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellMonthlyResource(TableLineInfo* tli)
{
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        double load = tli->resource->
            getLoad(tli->sc, Interval(month).firstMonth(),
                    AllAccounts, tli->task);
        QString bgCol =
            load > tli->resource->getMinEffort() * 
            tli->resource->getEfficiency() ?
            (tli->ca2 == 0 ? "booked" :
             (tli->task->isCompleted(tli->sc, sameTimeNextMonth(month) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameMonth(report->getProject()->getNow(), month) ? 
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellMonthlyAccount(TableLineInfo*)
{
}

void
HTMLReportElement::genCellQuarterlyTask(TableLineInfo* tli)
{
    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        double load = tli->task->getLoad(tli->sc, 
                                         Interval(quarter).firstQuarter(),
                                         tli->resource);
        QString bgCol = 
            tli->task->isActive(tli->sc, 
                                Interval(quarter).firstQuarter()) ?
            (tli->task->isCompleted(tli->sc, 
                                    sameTimeNextQuarter(quarter) - 1) ?
             (tli->ca2 == 0 ? "completed" : "completedlight"):
             tli->task->isMilestone() ? "milestone" :
             (tli->ca2 == 0 &&
              !tli->task->isBuffer(tli->sc, 
                                   Interval(quarter).firstQuarter())
              ? "booked" : "bookedlight")) :
            isSameQuarter(report->getProject()->getNow(), quarter) ? 
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->task->isContainer(), 
                   tli->task->isActive(tli->sc, 
                                       Interval(quarter).firstQuarter()) && 
                   tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellQuarterlyResource(TableLineInfo* tli)
{
    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        double load = tli->resource->
            getLoad(tli->sc, Interval(quarter).firstQuarter(),
                    AllAccounts, tli->task);
        QString bgCol =
            load > tli->resource->getMinEffort() * 
            tli->resource->getEfficiency() ?
            (tli->ca2 == 0 ? "booked" :
             (tli->task->isCompleted(tli->sc, 
                                     sameTimeNextQuarter(quarter) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameQuarter(report->getProject()->getNow(), quarter) ? 
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellQuarterlyAccount(TableLineInfo*)
{
}

void
HTMLReportElement::genCellYearlyTask(TableLineInfo* tli)
{
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        double load = tli->task->getLoad(tli->sc, Interval(year).firstYear(),
                                         tli->resource);
        QString bgCol = 
            tli->task->isActive(tli->sc, 
                                Interval(year).firstYear()) ?
            (tli->task->isCompleted(tli->sc, sameTimeNextYear(year) - 1) ?
             (tli->ca2 == 0 ? "completed" : "completedlight"):
             tli->task->isMilestone() ? "milestone" :
             (tli->ca2 == 0 &&
              !tli->task->isBuffer(tli->sc, 
                                   Interval(year).firstYear())
              ? "booked" : "bookedlight")) :
            isSameYear(report->getProject()->getNow(), year) ? 
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->task->isContainer(), 
                   tli->task->isActive(tli->sc, Interval(year).firstYear()) && 
                   tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellYearlyResource(TableLineInfo* tli)
{
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        double load = tli->resource->
            getLoad(tli->sc, Interval(year).firstYear(),
                    AllAccounts, tli->task);
        QString bgCol =
            load > tli->resource->getMinEffort() * 
            tli->resource->getEfficiency() ?
            (tli->ca2 == 0 ? "booked" :
             (tli->task->isCompleted(tli->sc, sameTimeNextYear(year) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameYear(report->getProject()->getNow(), year) ? 
            "today" : (tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellYearlyAccount(TableLineInfo*)
{
}

void
HTMLReportElement::genCellResponsibilities(TableLineInfo* tli)
{
    s() << "   <td class=\""
      << (tli->ca2 != 0 ? "defaultlight" : "default")
      << "\" rowspan=\""
      << QString("%1").arg(scenarios.count())
      << "\" style=\"text-align:left\">"
        "<span style=\"font-size:100%\">";
    bool first = TRUE;
    for (TaskListIterator it(report->getProject()->getTaskListIterator());
         *it != 0; ++it)
    {
        if ((*it)->getResponsible() == tli->resource)
        {
            if (!first)
                s() << ", ";
            s() << (*it)->getId();
            first = FALSE;
        }
    }
    s() << "</span></td>" << endl;
}

void
HTMLReportElement::genCellSchedule(TableLineInfo* tli)
{
    s() << "   <td class=\""
      << (tli->ca2 == 0 ? "default" : "defaultlight") 
      << "\" style=\"text-align:left\">";

    if (tli->resource)
    {
        BookingList jobs = tli->resource->getJobs(tli->sc);
        jobs.setAutoDelete(TRUE);
        time_t prevTime = 0;
        Interval reportPeriod(start, end);
        s() << "<table style=\"width:150px; font-size:100%; "
           "text-align:left\"><tr><th style=\"width:35%\"></th>"
           "<th style=\"width:65%\"></th></tr>" << endl;
        for (BookingListIterator bli(jobs); *bli != 0; ++bli)
        {
            if ((tli->ca2 == 0 || tli->task == (*bli)->getTask()) && 
                reportPeriod.overlaps(Interval((*bli)->getStart(), 
                                               (*bli)->getEnd())))
            {
                /* If the reporting interval is not more than a day, we
                 * do not print the day since this information is most
                 * likely given by the context of the report. */
                if (!isSameDay(prevTime, (*bli)->getStart()) &&
                    !isSameDay(start, end - 1))
                {
                    s() << "<tr><td colspan=\"2\" style=\"font-size:120%\">"
                        << time2weekday((*bli)->getStart()) << ", "
                        << time2date((*bli)->getStart()) << "</td></tr>" 
                        << endl;
                }
                s() << "<tr><td>";
                Interval workPeriod((*bli)->getStart(), (*bli)->getEnd());
                workPeriod.overlap(reportPeriod);
                s() << time2user(workPeriod.getStart(), shortTimeFormat)
                    << "&nbsp;-&nbsp;"
                    << time2user(workPeriod.getEnd() + 1, shortTimeFormat);
                s() << "</td><td>";
                if (tli->ca2 == 0)
                    s() << " " << htmlFilter((*bli)->getTask()->getName());
                s() << "</td>" << endl;
                prevTime = (*bli)->getStart();
                s() << "</tr>" << endl;
            }
        }
        s() << "</table>" << endl;
    }
    else
        s() << "&nbsp;";

    s() << "</td>" << endl;
}

#define GCEFFORT(a) \
void \
HTMLReportElement::genCell##a##Effort(TableLineInfo* tli) \
{ \
    textMultiRows(QString().sprintf("%.2f", tli->resource->get##a##Effort()), \
                  tli->ca2 != 0, "right"); \
}

GCEFFORT(Min)
GCEFFORT(Max)

void
HTMLReportElement::genCellRate(TableLineInfo* tli)
{
    textMultiRows(QString()
                  .sprintf("%.*f", report->getProject()->getCurrencyDigits(),
                           tli->resource->getRate()),
                  tli->ca2 != 0, "right");
}

void
HTMLReportElement::genCellKotrusId(TableLineInfo* tli)
{
    textMultiRows(tli->resource->getKotrusId(), tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellTotal(TableLineInfo* tli)
{
    double value = tli->account->getVolume(tli->sc, Interval(start, end));
    columnTotals[tli->sc]["total"] += value;
    textOneRow(QString().sprintf("%.*f",
                                 report->getProject()->getCurrencyDigits(),
                                 value), FALSE, "right");
}

void
HTMLReportElement::genCellSummary(TableLineInfo* tli)
{
    multiRowCell("&nbsp", tli);
}

