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
HTMLReportElement::generateFirstTask(int sc, const Task* t, const Resource* r, 
                                     uint no)
{
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" 
        << colors.getColor("default").light(r ? 120 : 100).name() 
        << "\">" << endl;
   
    TableLineInfo tli(t, r, t, r, 0, no, sc);

    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genTaskLine1)
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
            (*this.*(columnFormat[(*it)->getName()]->genTaskLine1))
                (&tci);
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
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" 
        << colors.getColor("default").light(r ? 120 : 100).name() 
        << "\">" << endl;
    
    TableLineInfo tli(t, r, t, r, 0, 0, sc);

    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genTaskLine2)
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
            (*this.*(columnFormat[(*it)->getName()]->genTaskLine2))
                (&tci);
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateFirstResource(int sc, const Resource* r, 
                                         const Task* t, uint no)
{
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" 
        << colors.getColor("default").light(t ? 120 : 100).name() 
        << "\">" << endl;

    TableLineInfo tli(r, t, t, r, 0, no, sc);
    
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genResourceLine1)
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
            (*this.*(columnFormat[(*it)->getName()]->genResourceLine1))
                (&tci);
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
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" 
        << colors.getColor("default").light(t ? 120 : 100).name() 
        << "\">" << endl;

    TableLineInfo tli(r, t, t, r, 0, 0, sc);
    
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genResourceLine2)
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
            (*this.*(columnFormat[(*it)->getName()]->genResourceLine2))
                (&tci);
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateFirstAccount(int sc, const Account* a, uint no)
{
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" << colors.getColorName("default") 
        << "\">" << endl;

    TableLineInfo tli(a, 0, 0, 0, a, no, sc);

    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genAccountLine1)
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
            (*this.*(columnFormat[(*it)->getName()]->genAccountLine1))
                (&tci);
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
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" << colors.getColorName("default") 
        << "\">" << endl;

    TableLineInfo tli(a, 0, 0, 0, a, 0, sc);
    
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()] &&
            columnFormat[(*it)->getName()]->genAccountLine2)
        {
            TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
            (*this.*(columnFormat[(*it)->getName()]->genAccountLine2))
                (&tci);
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
        << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" << colors.getColorName("header") << "; "
        << "font-size:110%; font-weight:bold; text-align:center\"" 
        << ">" << endl;
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
    bool td = FALSE;
    s() << "  <tr>" << endl;
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        if (columnFormat[(*it)->getName()])
            if (columnFormat[(*it)->getName()]->genHeadLine2)
            {
                TableCellInfo tci(columnFormat[(*it)->getName()], 0, *it);
                (*this.*(columnFormat[(*it)->getName()]->genHeadLine2))
                    (&tci);
                td = TRUE;
            }
    }
    if (!td)
        s() << "   <td>&nbsp;</td>" << endl;
    s() << "  </tr>" << endl << " </thead>\n" << endl;
}

void
HTMLReportElement::generateSummaryFirst(int sc, const QString& name, 
                                        const QString& bgCol)
{
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" << bgCol << "; "
        << "font-weight:bold\"" << ">" << endl;
    
    TableLineInfo tli(0, 0, 0, 0, 0, 0, sc);
    
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
        if ((*it)->getName() == "name")
            genCell(name, &tci, TRUE); 
        else if (columnFormat[(*it)->getName()])
        {
            if (columnFormat[(*it)->getName()]->genSummaryLine1)
            {
                (*this.*(columnFormat[(*it)->getName()]->genSummaryLine1))
                    (&tci);
            }
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::generateSummaryNext(int sc, const QString& bgCol)
{
    s() << "  <tr valign=\"middle\"" 
        << " style=\"background-color:" << bgCol << "; "
        << "font-weight:bold\"" << ">" << endl;

    TableLineInfo tli(0, 0, 0, 0, 0, 0, sc);
    
    for (QPtrListIterator<TableColumnInfo> it(columns); it; ++it )
    {
        TableCellInfo tci(columnFormat[(*it)->getName()], &tli, *it);
        if (columnFormat[(*it)->getName()])
        {
            if (columnFormat[(*it)->getName()]->genSummaryLine2)
            {
                (*this.*(columnFormat[(*it)->getName()]->genSummaryLine2))
                    (&tci);
            }
        }
    }
    s() << "  </tr>" << endl;
}

void
HTMLReportElement::genCell(const QString& text, TableCellInfo* tci, bool multi)
{
    s() << "   <td";
    if (multi && scenarios.count() > 1)
        s() << " rowspan=\"" << QString("%1").arg(scenarios.count()) << "\"";
    if (!tci->tcf->hAlign.isEmpty())
    {
        s() << " style=\"";
        if (!tci->tcf->hAlign.isEmpty())
            s() << "text-align:" << tci->tcf->hAlign << "; ";
        s() << "\"";
    }
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
HTMLReportElement::genHeadDefault(TableCellInfo* tci)
{
    s() << "   <td rowspan=\"2\">"
        << htmlFilter(tci->tcf->getTitle()) << "</td>" << endl;
}

void
HTMLReportElement::genHeadCurrency(TableCellInfo* tci)
{
    s() << "   <td class=\"headerbig\" rowspan=\"2\">"
        << htmlFilter(i18n(tci->tcf->getTitle()));
    if (!report->getProject()->getCurrency().isEmpty())
        s() << " " << htmlFilter(report->getProject()->getCurrency());
    s() << "</td>" << endl;
}

void
HTMLReportElement::genHeadDaily1(TableCellInfo*)
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
HTMLReportElement::genHeadDaily2(TableCellInfo*)
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
HTMLReportElement::genHeadWeekly1(TableCellInfo*)
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
HTMLReportElement::genHeadWeekly2(TableCellInfo*)
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
HTMLReportElement::genHeadMonthly1(TableCellInfo*)
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
HTMLReportElement::genHeadMonthly2(TableCellInfo*)
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
HTMLReportElement::genHeadQuarterly1(TableCellInfo*)
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
HTMLReportElement::genHeadQuarterly2(TableCellInfo*)
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
HTMLReportElement::genHeadYear(TableCellInfo*)
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
HTMLReportElement::genCellEmpty(TableCellInfo* tci)
{
    genCell("&nbsp;", tci, TRUE);
}

void
HTMLReportElement::genCellSequenceNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? 
            QString().sprintf("%d.", tci->tli->ca1->getSequenceNo()) :
            QString("&nbsp;"), tci, TRUE);
}

void
HTMLReportElement::genCellNo(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? QString().sprintf("%d.", tci->tli->no) :
            QString("&nbsp;"), tci, TRUE);
}

void
HTMLReportElement::genCellIndex(TableCellInfo* tci)
{
    genCell(tci->tli->ca2 == 0 ? 
            QString().sprintf("%d.", tci->tli->ca1->getIndex()) :
            QString("&nbsp;"), tci, TRUE);
}

void
HTMLReportElement::genCellId(TableCellInfo* tci)
{
    textMultiRows(htmlFilter(tci->tli->ca1->getId()), tci->tli->ca2 != 0, 
                  "left");
}

void
HTMLReportElement::genCellName(TableCellInfo* tci)
{
    int lPadding = 0;
    int fontSize = tci->tli->ca2 == 0 ? 100 : 90; 
    if ((tci->tli->ca2 && (strcmp(tci->tli->ca2->getType(), "Resource") == 0 &&
          resourceSortCriteria[0] == CoreAttributesList::TreeMode)) ||
        (tci->tli->ca2 && strcmp(tci->tli->ca2->getType(), "Task") == 0 &&
         taskSortCriteria[0] == CoreAttributesList::TreeMode))
        for (const CoreAttributes* cp = tci->tli->ca2 ; cp != 0;
             cp = cp->getParent())
            lPadding++;

    mt.clear();
    if (tci->tli->task)
        mt.addMacro(new Macro(KW("taskid"), tci->tli->task->getId(), 
                              defFileName, defFileLine));
    if (tci->tli->resource)
        mt.addMacro(new Macro(KW("resourceid"), tci->tli->resource->getId(),
                              defFileName, defFileLine));
    if (tci->tli->account)
        mt.addMacro(new Macro(KW("accountid"), tci->tli->account->getId(),
                              defFileName, defFileLine));

    if ((strcmp(tci->tli->ca1->getType(), "Resource") == 0 &&
         resourceSortCriteria[0] == CoreAttributesList::TreeMode) ||
        (strcmp(tci->tli->ca1->getType(), "Task") == 0 &&
         taskSortCriteria[0] == CoreAttributesList::TreeMode) ||
        (strcmp(tci->tli->ca1->getType(), "Account") == 0 &&
         accountSortCriteria[0] == CoreAttributesList::TreeMode))
    {
        lPadding += tci->tli->ca1->treeLevel();
        fontSize = fontSize + 5 * (maxDepthTaskList - 1 - 
                                   tci->tli->ca1->treeLevel()); 
        s() << "   <td class=\""
          << (tci->tli->ca2 == 0 ? "task" : "tasklight") << "\" rowspan=\""
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
          << (tci->tli->ca2 == 0 ? "task" : "tasklight") << "\" rowspan=\""
          << QString("%1").arg(scenarios.count())
          << "\" style=\"white-space:nowrap;"
          << " padding-left: " 
          << QString("%1").arg(2 + lPadding * 15)
          << ";\"><span>";
    }
    if (strcmp(tci->tli->ca1->getType(), "Task") == 0)
    {
        if (tci->tli->ca2 == 0)
            s() << "<a name=\"task_" << tci->tli->ca1->getId() << "\"></a>";
        s() << generateUrl(KW("taskname"), tci->tli->ca1->getName());
    }
    else if (strcmp(tci->tli->ca1->getType(), "Resource") == 0)
    {
        if (tci->tli->ca2 == 0)
            s() << "<a name=\"resource_" << tci->tli->ca1->getFullId() 
                << "\"></a>";
        s() << generateUrl(KW("resourcename"), tci->tli->ca1->getName());
    }
    else if (strcmp(tci->tli->ca1->getType(), "Account") == 0)
    {
        if (tci->tli->ca2 == 0)
            s() << "<a name=\"account_" << tci->tli->ca1->getId() << "\"></a>";
        s() << generateUrl(KW("accountname"), tci->tli->ca1->getName());
    }
    s() << "</span></td>" << endl;
}

#define GCSE(a) \
void \
HTMLReportElement::genCell##a(TableCellInfo* tci) \
{ \
    s() << "   <td class=\"" \
        << (tci->tli->task->is##a##Ok(tci->tli->sc) ? \
            (tci->tli->resource == 0 ? "default" : "defaultlight") : \
            "milestone") \
        << "\" style=\"text-align:left white-space:nowrap\">" \
        << time2user(tci->tli->task->get##a(tci->tli->sc), timeFormat) \
        << "</td>" << endl; \
}

GCSE(Start)
GCSE(End)
    
#define GCMMSE(a) \
void \
HTMLReportElement::genCell##a(TableCellInfo* tci) \
{ \
    textMultiRows(tci->tli->task->get##a() == 0 ? "&nbsp;" : \
                  time2user(tci->tli->task->get##a(), timeFormat), \
                  tci->tli->resource != 0, "&nbsp;"); \
}

GCMMSE(MinStart)
GCMMSE(MaxStart)
GCMMSE(MinEnd)
GCMMSE(MaxEnd)

#define GCSEBUFFER(a) \
void \
HTMLReportElement::genCell##a##Buffer(TableCellInfo* tci) \
{ \
    textOneRow(QString().sprintf \
               ("%3.0f", tci->tli->task->get##a##Buffer(tci->tli->sc)), \
               tci->tli->resource != 0, "right"); \
}

GCSEBUFFER(Start)
GCSEBUFFER(End)

void
HTMLReportElement::genCellStartBufferEnd(TableCellInfo* tci)
{
    textOneRow(time2user(tci->tli->task->getStartBufferEnd
                         (tci->tli->sc), timeFormat), tci->tli->resource != 0, 
               "left");
}

void
HTMLReportElement::genCellEndBufferStart(TableCellInfo* tci)
{
    textOneRow(time2user(tci->tli->task->getStartBufferEnd
                         (tci->tli->sc) + 1, timeFormat), 
               tci->tli->resource != 0, "left");
}

void
HTMLReportElement::genCellDuration(TableCellInfo* tci)
{
    s() << "   <td class=\""
        << (tci->tli->resource == 0 ? "default" : "defaultlight")
        << "\" style=\"text-align:right white-space:nowrap\">"
        << scaledLoad(tci->tli->task->getCalcDuration(tci->tli->sc))
        << "</td>" << endl;
}

void
HTMLReportElement::genCellEffort(TableCellInfo* tci)
{
    double val = 0.0;
    if (strcmp(tci->tli->ca1->getType(), "Task") == 0)
    {
        val = tci->tli->task->getLoad(tci->tli->sc, Interval(start, end),
                                 tci->tli->resource);
    }
    else if (strcmp(tci->tli->ca1->getType(), "Resource") == 0)
    {
        val = tci->tli->resource->getLoad(tci->tli->sc, Interval(start, end), 
                                     AllAccounts, tci->tli->task);
    }
    
    generateRightIndented(tci->tli, scaledLoad(val));
}

void
HTMLReportElement::genCellProjectId(TableCellInfo* tci)
{
    textMultiRows(tci->tli->task->getProjectId() + " (" +
                  report->getProject()->getIdIndex(tci->tli->task->
                                                   getProjectId()) + ")",
                  tci->tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellResources(TableCellInfo* tci)
{
    s() << "   <td class=\""
      << (tci->tli->ca2 != 0 ? "defaultlight" : "default")
      << "\" style=\"text-align:left\">"
      << "<span style=\"font-size:100%\">";
    bool first = TRUE;
    for (ResourceListIterator rli(tci->tli->task->
                                  getBookedResourcesIterator(tci->tli->sc));
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
HTMLReportElement::genCellResponsible(TableCellInfo* tci)
{
    if (tci->tli->task->getResponsible())
        textMultiRows(htmlFilter(tci->tli->task->getResponsible()->getName()),
                      tci->tli->ca2 != 0, "left");
    else
        textMultiRows("&nbsp", tci->tli->ca2 != 0, "left");
}

#define GCNOTE(a, b) \
void \
HTMLReportElement::genCell##a##Note(TableCellInfo* tci) \
{ \
    s() << "   <td class=\"" \
        << (tci->tli->ca2 == 0 ? "default" : "defaultlight") \
        << "\" rowspan=\"" \
        << QString("%1").arg(scenarios.count()) \
        << "\" style=\"text-align:left\">" \
        << "<span style=\"font-size:100%\">"; \
    if (tci->tli->task->get##a##Note(b).isEmpty()) \
        s() << "&nbsp;"; \
    else \
        s() << htmlFilter(tci->tli->task->get##a##Note(b)); \
    s() << "</span></td>" << endl; \
}

GCNOTE(, )
GCNOTE(Status, tci->tli->sc)

void
HTMLReportElement::genCellCost(TableCellInfo* tci)
{
    double val = 0.0;
    if (strcmp(tci->tli->ca1->getType(), "Task") == 0)
    {
        val = tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                       Cost, tci->tli->resource);
    }
    else if (strcmp(tci->tli->ca1->getType(), "Resource") == 0)
    {
        val = tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                        Cost, tci->tli->task);
    }
    generateRightIndented
        (tci->tli, QString().sprintf("%.*f", report->getProject()->
                                     getCurrencyDigits(), val)); 
}

void
HTMLReportElement::genCellRevenue(TableCellInfo* tci)
{
    double val = 0.0;
    if (strcmp(tci->tli->ca1->getType(), "Task") == 0)
    {
        val = tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                       Revenue, tci->tli->resource);
    }
    else if (strcmp(tci->tli->ca1->getType(), "Resource") == 0)
    {
        val = tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                        Revenue, tci->tli->task);
    }
    generateRightIndented
        (tci->tli, QString().sprintf("%.*f", report->getProject()->
                                     getCurrencyDigits(), val)); }

void
HTMLReportElement::genCellProfit(TableCellInfo* tci)
{
    double val = 0.0;
    if (strcmp(tci->tli->ca1->getType(), "Task") == 0)
    {
        val = tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                    Revenue, tci->tli->resource) - 
            tci->tli->task->getCredits(tci->tli->sc, Interval(start, end),
                                  Cost, tci->tli->resource);
    }
    else if (strcmp(tci->tli->ca1->getType(), "Resource") == 0)
    {
        val = tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                        Revenue, tci->tli->task) -
            tci->tli->resource->getCredits(tci->tli->sc, Interval(start, end),
                                      Cost, tci->tli->task);
    }
    generateRightIndented(tci->tli, 
                          QString().sprintf("%.*f", report->getProject()->
                                            getCurrencyDigits(), val)); 
}

void
HTMLReportElement::genCellPriority(TableCellInfo* tci)
{
    textMultiRows(QString().sprintf("%d", tci->tli->task->getPriority()),
                  tci->tli->ca2 != 0, "right");
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
        flagStr += htmlFilter(*it);
    }
    if (flagStr.isEmpty())
        flagStr = "&nbsp";
    textMultiRows(flagStr, tci->tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellCompleted(TableCellInfo* tci)
{
    if (tci->tli->task->getCompletionDegree(tci->tli->sc) ==
        tci->tli->task->getCalcedCompletionDegree(tci->tli->sc))
    {
        textOneRow(QString("%1%").arg((int) tci->tli->task->
                                      getCompletionDegree(tci->tli->sc)),
                   tci->tli->ca2 != 0, "right");
    }
    else
    {
        textOneRow
            (QString("%1% (%2%)")
             .arg((int) tci->tli->task->getCompletionDegree(tci->tli->sc))
             .arg((int) tci->tli->task->
                  getCalcedCompletionDegree(tci->tli->sc)),
             tci->tli->ca2 != 0, "right");
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
    textOneRow(text, tci->tli->ca2 != 0, "center");
}

void
HTMLReportElement::genCellReference(TableCellInfo* tci)
{
    s() << "   <td class=\""
        << (tci->tli->ca2 == 0 ? "default" : "defaultlight")
        << "\" rowspan=\""
        << QString("%1").arg(scenarios.count())
        << "\" style=\"text-align:left\">"
        << "<span style=\"font-size:100%\">";
    if (tci->tli->task->getReference().isEmpty())
        s() << "&nbsp;";
    else
    {
        s() << "<a href=\"" << tci->tli->task->getReference() << "\">";
        if (tci->tli->task->getReferenceLabel().isEmpty())
            s() << htmlFilter(tci->tli->task->getReference());
        else
            s() << htmlFilter(tci->tli->task->getReferenceLabel());
        s() << "<a>";
    }
    s() << "</span></td>" << endl;
}

void
HTMLReportElement::genCellScenario(TableCellInfo* tci)
{
    genCell(htmlFilter(report->getProject()->
                       getScenarioName(tci->tli->sc)), tci, FALSE);
}

#define GCDEPFOL(a, b) \
void \
HTMLReportElement::genCell##a(TableCellInfo* tci) \
{ \
    s() << "   <td class=\"" \
      << (tci->tli->ca2 != 0 ? "defaultlight" : "default") \
      << "\" rowspan=\"" \
      << QString("%1").arg(scenarios.count()) \
      << "\" style=\"text-align:left\"><span style=\"font-size:100%\">"; \
    bool first = TRUE; \
    for (TaskListIterator it(tci->tli->task->get##b##Iterator()); *it != 0; \
         ++it) \
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
HTMLReportElement::genCellDailyTask(TableCellInfo* tci)
{
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextDay(day))
    {
        double load = tci->tli->task->getLoad(tci->tli->sc, 
                                              Interval(day).firstDay(),
                                         tci->tli->resource);
        QString bgCol = 
            tci->tli->task->isActive(tci->tli->sc, Interval(day).firstDay()) ? 
            (tci->tli->task->isCompleted(tci->tli->sc,
                                    sameTimeNextDay(day) - 1) ?
             (tci->tli->ca2 == 0 ? "completed" : "completedlight") :
             tci->tli->task->isMilestone() ? "milestone" :
             (tci->tli->ca2 == 0 && !tci->tli->task->isBuffer(tci->tli->sc, 
                                                    Interval(day).firstDay())
              ? "booked" : "bookedlight")) :
            isSameDay(report->getProject()->getNow(), day) ? "today" :
            isWeekend(day) ? "weekend" :
            report->getProject()->isVacation(day) ? "vacation" :
            (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        if (showPIDs)
        {
            QString pids = tci->tli->resource->
                getProjectIDs(tci->tli->sc, Interval(day).firstDay(), 
                              tci->tli->task);
            reportPIDs(pids, bgCol, !tci->tli->resource->isGroup());
        }
        else
            reportLoad(load, bgCol, !tci->tli->task->isContainer(),
                       tci->tli->task->isActive(tci->tli->sc, 
                                                Interval(day).firstDay()) &&
                       tci->tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellDailyResource(TableCellInfo* tci)
{
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextDay(day))
    {
        double load = tci->tli->resource->getLoad(tci->tli->sc, 
                                                  Interval(day).firstDay(),
                                             AllAccounts, tci->tli->task);
        QString bgCol = 
            load > tci->tli->resource->getMinEffort() * 
            tci->tli->resource->getEfficiency() ?
            (tci->tli->ca2 == 0 ? "booked" :
             (tci->tli->task->isCompleted(tci->tli->sc, 
                                     sameTimeNextDay(day) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameDay(report->getProject()->getNow(), day) ? "today" :
            isWeekend(day) ? "weekend" :
            report->getProject()->isVacation(day) || 
            tci->tli->resource->hasVacationDay(day) ?
            "vacation" :
            (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellDailyAccount(TableCellInfo* tci)
{
    for (time_t day = midnight(start); day < end;
         day = sameTimeNextDay(day))
    {
        QString style("default");
        double volume = tci->tli->account->getVolume(tci->tli->sc, 
                                                     Interval(day).firstDay());
        if (tci->tli->account->isLeaf())
            tci->tci->addToSum(tci->tli->sc, time2ISO(day), volume);
        reportValue(volume, style, FALSE);
    }
}

void
HTMLReportElement::genCellWeeklyTask(TableCellInfo* tci)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        double load = tci->tli->task->getLoad(tci->tli->sc, Interval(week).
                                         firstWeek(weekStartsMonday),
                                         tci->tli->resource);
        QString bgCol = 
            tci->tli->task->isActive(tci->tli->sc, 
                                Interval(week).firstWeek(weekStartsMonday)) ?
            (tci->tli->task->isCompleted(tci->tli->sc, 
                                         sameTimeNextWeek(week) - 1) ?
             (tci->tli->ca2 == 0 ? "completed" : "completedlight") :
             tci->tli->task->isMilestone() ? "milestone" :
             (tci->tli->ca2 == 0 && 
              !tci->tli->task->isBuffer(tci->tli->sc, Interval(week).
                                   firstWeek(weekStartsMonday))
              ? "booked" : "bookedlight")) :
            isSameWeek(report->getProject()->getNow(), week, 
                       weekStartsMonday) ?
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->task->isContainer(), 
                   tci->tli->task->isActive(tci->tli->sc, 
                                       Interval(week).
                                       firstWeek(weekStartsMonday)) && 
                   tci->tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellWeeklyResource(TableCellInfo* tci)
{
    bool weekStartsMonday = report->getWeekStartsMonday();
    for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
         week = sameTimeNextWeek(week))
    {
        double load = 
            tci->tli->resource->getLoad(tci->tli->sc, 
                                   Interval(week).firstWeek(weekStartsMonday),
                                   AllAccounts, tci->tli->task);
        QString bgCol =
            load > tci->tli->resource->getMinEffort() * 
            tci->tli->resource->getEfficiency() ?
            (tci->tli->ca2 == 0 ? "booked" :
             (tci->tli->task->isCompleted(tci->tli->sc, 
                                          sameTimeNextWeek(week) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameWeek(report->getProject()->getNow(), week, 
                       weekStartsMonday) ?
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        if (showPIDs)
        {
            QString pids = tci->tli->resource->
                getProjectIDs(tci->tli->sc, 
                              Interval(week).firstWeek(weekStartsMonday), 
                              tci->tli->task);
            reportPIDs(pids, bgCol, !tci->tli->resource->isGroup());
        }
        else
            reportLoad(load, bgCol, !tci->tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellWeeklyAccount(TableCellInfo*)
{
}

void
HTMLReportElement::genCellMonthlyTask(TableCellInfo* tci)
{
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        double load = tci->tli->task->getLoad(tci->tli->sc, 
                                              Interval(month).firstMonth(),
                                         tci->tli->resource);
        QString bgCol = 
            tci->tli->task->isActive(tci->tli->sc, 
                                Interval(month).firstMonth()) ?
            (tci->tli->task->isCompleted(tci->tli->sc, 
                                    sameTimeNextMonth(month) - 1) ?
             (tci->tli->ca2 == 0 ? "completed" : "completedlight"):
             tci->tli->task->isMilestone() ? "milestone" :
             (tci->tli->ca2 == 0 &&
              !tci->tli->task->isBuffer(tci->tli->sc, 
                                   Interval(month).firstMonth())
              ? "booked" : "bookedlight")) :
            isSameMonth(report->getProject()->getNow(), month) ? 
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->task->isContainer(), 
                   tci->tli->task->isActive(tci->tli->sc, 
                                            Interval(month).firstMonth()) && 
                   tci->tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellMonthlyResource(TableCellInfo* tci)
{
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        double load = tci->tli->resource->
            getLoad(tci->tli->sc, Interval(month).firstMonth(),
                    AllAccounts, tci->tli->task);
        QString bgCol =
            load > tci->tli->resource->getMinEffort() * 
            tci->tli->resource->getEfficiency() ?
            (tci->tli->ca2 == 0 ? "booked" :
             (tci->tli->task->isCompleted(tci->tli->sc, 
                                          sameTimeNextMonth(month) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameMonth(report->getProject()->getNow(), month) ? 
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellMonthlyAccount(TableCellInfo* tci)
{
    for (time_t month = beginOfMonth(start); month < end;
         month = sameTimeNextMonth(month))
    {
        QString style("default");
        double volume = tci->tli->account->
            getVolume(tci->tli->sc, Interval(month).firstDay());
        if (tci->tli->account->isLeaf())
            tci->tci->addToSum(tci->tli->sc, time2ISO(month), volume);
        reportValue(volume, style, FALSE);
    }
}

void
HTMLReportElement::genCellQuarterlyTask(TableCellInfo* tci)
{
    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        double load = tci->tli->task->getLoad(tci->tli->sc, 
                                         Interval(quarter).firstQuarter(),
                                         tci->tli->resource);
        QString bgCol = 
            tci->tli->task->isActive(tci->tli->sc, 
                                Interval(quarter).firstQuarter()) ?
            (tci->tli->task->isCompleted(tci->tli->sc, 
                                    sameTimeNextQuarter(quarter) - 1) ?
             (tci->tli->ca2 == 0 ? "completed" : "completedlight"):
             tci->tli->task->isMilestone() ? "milestone" :
             (tci->tli->ca2 == 0 &&
              !tci->tli->task->isBuffer(tci->tli->sc, 
                                   Interval(quarter).firstQuarter())
              ? "booked" : "bookedlight")) :
            isSameQuarter(report->getProject()->getNow(), quarter) ? 
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->task->isContainer(), 
                   tci->tli->task->isActive(tci->tli->sc, 
                                       Interval(quarter).firstQuarter()) && 
                   tci->tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellQuarterlyResource(TableCellInfo* tci)
{
    for (time_t quarter = beginOfQuarter(start); quarter < end;
         quarter = sameTimeNextQuarter(quarter))
    {
        double load = tci->tli->resource->
            getLoad(tci->tli->sc, Interval(quarter).firstQuarter(),
                    AllAccounts, tci->tli->task);
        QString bgCol =
            load > tci->tli->resource->getMinEffort() * 
            tci->tli->resource->getEfficiency() ?
            (tci->tli->ca2 == 0 ? "booked" :
             (tci->tli->task->isCompleted(tci->tli->sc, 
                                     sameTimeNextQuarter(quarter) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameQuarter(report->getProject()->getNow(), quarter) ? 
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellQuarterlyAccount(TableCellInfo*)
{
}

void
HTMLReportElement::genCellYearlyTask(TableCellInfo* tci)
{
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        double load = tci->tli->task->getLoad(tci->tli->sc, 
                                              Interval(year).firstYear(),
                                         tci->tli->resource);
        QString bgCol = 
            tci->tli->task->isActive(tci->tli->sc, 
                                Interval(year).firstYear()) ?
            (tci->tli->task->isCompleted(tci->tli->sc, 
                                         sameTimeNextYear(year) - 1) ?
             (tci->tli->ca2 == 0 ? "completed" : "completedlight"):
             tci->tli->task->isMilestone() ? "milestone" :
             (tci->tli->ca2 == 0 &&
              !tci->tli->task->isBuffer(tci->tli->sc, 
                                   Interval(year).firstYear())
              ? "booked" : "bookedlight")) :
            isSameYear(report->getProject()->getNow(), year) ? 
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->task->isContainer(), 
                   tci->tli->task->isActive(tci->tli->sc, 
                                            Interval(year).firstYear()) && 
                   tci->tli->task->isMilestone());
    }
}

void
HTMLReportElement::genCellYearlyResource(TableCellInfo* tci)
{
    for (time_t year = beginOfYear(start); year < end;
         year = sameTimeNextYear(year))
    {
        double load = tci->tli->resource->
            getLoad(tci->tli->sc, Interval(year).firstYear(),
                    AllAccounts, tci->tli->task);
        QString bgCol =
            load > tci->tli->resource->getMinEffort() * 
            tci->tli->resource->getEfficiency() ?
            (tci->tli->ca2 == 0 ? "booked" :
             (tci->tli->task->isCompleted(tci->tli->sc, 
                                          sameTimeNextYear(year) - 1) ?
              "completedlight" : "bookedlight")) :
            isSameYear(report->getProject()->getNow(), year) ? 
            "today" : (tci->tli->ca2 == 0 ? "default" : "defaultlight");
        reportLoad(load, bgCol, !tci->tli->resource->isGroup());
    }
}

void
HTMLReportElement::genCellYearlyAccount(TableCellInfo*)
{
}

void
HTMLReportElement::genCellResponsibilities(TableCellInfo* tci)
{
    s() << "   <td class=\""
      << (tci->tli->ca2 != 0 ? "defaultlight" : "default")
      << "\" rowspan=\""
      << QString("%1").arg(scenarios.count())
      << "\" style=\"text-align:left\">"
        "<span style=\"font-size:100%\">";
    bool first = TRUE;
    for (TaskListIterator it(report->getProject()->getTaskListIterator());
         *it != 0; ++it)
    {
        if ((*it)->getResponsible() == tci->tli->resource)
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
HTMLReportElement::genCellSchedule(TableCellInfo* tci)
{
    s() << "   <td class=\""
      << (tci->tli->ca2 == 0 ? "default" : "defaultlight") 
      << "\" style=\"text-align:left\">";

    if (tci->tli->resource)
    {
        BookingList jobs = tci->tli->resource->getJobs(tci->tli->sc);
        jobs.setAutoDelete(TRUE);
        time_t prevTime = 0;
        Interval reportPeriod(start, end);
        s() << "<table style=\"width:150px; font-size:100%; "
           "text-align:left\"><tr><th style=\"width:35%\"></th>"
           "<th style=\"width:65%\"></th></tr>" << endl;
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
                if (tci->tli->ca2 == 0)
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
HTMLReportElement::genCell##a##Effort(TableCellInfo* tci) \
{ \
    textMultiRows(QString().sprintf("%.2f", tci->tli->resource->get##a##Effort()), \
                  tci->tli->ca2 != 0, "right"); \
}

GCEFFORT(Min)
GCEFFORT(Max)

void
HTMLReportElement::genCellRate(TableCellInfo* tci)
{
    textMultiRows(QString()
                  .sprintf("%.*f", report->getProject()->getCurrencyDigits(),
                           tci->tli->resource->getRate()),
                  tci->tli->ca2 != 0, "right");
}

void
HTMLReportElement::genCellKotrusId(TableCellInfo* tci)
{
    textMultiRows(tci->tli->resource->getKotrusId(), tci->tli->ca2 != 0, "left");
}

void
HTMLReportElement::genCellTotal(TableCellInfo* tci)
{
    double value = tci->tli->account->getVolume(tci->tli->sc, 
                                                Interval(start, end));
    if (tci->tli->account->isLeaf())
        tci->tci->addToSum(tci->tli->sc, "total", value);
    genCell(QString().sprintf("%.*f",
                              report->getProject()->getCurrencyDigits(),
                              value), tci, FALSE);
}

void
HTMLReportElement::genCellSummary(TableCellInfo* tci)
{
    QMap<QString, double>::ConstIterator it;
    const QMap<QString, double>* sum = tci->tci->getSum();
    if (sum)
    {
        uint sc = tci->tli->sc;
        for (it = sum[sc].begin(); it != sum[sc].end(); ++it)
            genCell(QString().sprintf
                    ("%.*f", report->getProject()->getCurrencyDigits(), 
                     *it), tci, FALSE);
    }
    else
        genCell("&nbsp;", tci, FALSE);
}

