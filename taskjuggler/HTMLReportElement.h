/*
 * HTMLReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLReportElement_h_
#define _HTMLReportElement_h_

#include <qstring.h>
#include <qmap.h>

#include "ReportElement.h"
#include "HTMLPrimitives.h"
#include "MacroTable.h"

class Project;
class Report;
class ExpressionTree;
class TableLineInfo;
class TableCellInfo;

/**
 * @short Models the basic component of an HTML report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLReportElement : public ReportElement, public HTMLPrimitives
{
public:
    HTMLReportElement(Report* r, const QString& df, int dl);
    virtual ~HTMLReportElement() { }

    enum BarLabelText { BLT_EMPTY = 0, BLT_LOAD };

    void generateTableHeader();

    void generateHeader();
    void generateFooter();

    void generateFirstTask(int sc, const Task* t, const Resource* r, uint no);
    void generateNextTask(int sc, const Task* t, const Resource* r);

    void generateFirstResource(int sc, const Resource* r, const Task* t, 
                               uint no);
    void generateNextResource(int sc, const Resource* r, const Task* t);

    void generateFirstAccount(int sc, const Account* a, uint no);
    void generateNextAccount(int sc, const Account* a);

    void generateSummaryFirst(int sc, const QString& name, 
                              const QString& style);
    void generateSummaryNext(int sc, const QString& style);

    void genCell(const QString& text, TableCellInfo* tli, bool multi);

    void textOneRow(const QString& text, bool light, const QString& align);
    void textMultiRows(const QString& text, bool light, const QString& align);

    void reportPIDs(const QString& pids, const QString bgCol, bool bold);

    void reportLoad(double load, const QString& bgcol, bool bold,
                    bool milestone = FALSE);

    void reportValue(double value, const QString& bgCol, bool bold);

    void setBarLabels(BarLabelText blt) { barLabels = blt; }

    void registerUrl(const QString& key, const QString& url = QString::null)
    {
        urls[key] = url;
    }
    void setRawHead(const QString& head)
    {
        rawHead = head;
    }

    void setRawTail(const QString& tail)
    {
        rawTail = tail;
    }

    virtual void genHeadDefault(TableCellInfo* tcf);
    virtual void genHeadCurrency(TableCellInfo* tcf);
    virtual void genHeadDaily1(TableCellInfo* tcf);
    virtual void genHeadDaily2(TableCellInfo* tcf);
    virtual void genHeadWeekly1(TableCellInfo* tcf);
    virtual void genHeadWeekly2(TableCellInfo* tcf);
    virtual void genHeadMonthly1(TableCellInfo* tcf);
    virtual void genHeadMonthly2(TableCellInfo* tcf);
    virtual void genHeadQuarterly1(TableCellInfo* tcf);
    virtual void genHeadQuarterly2(TableCellInfo* tcf);
    virtual void genHeadYear(TableCellInfo* tcf);
   
    virtual void genCellEmpty(TableCellInfo*); 
    virtual void genCellSequenceNo(TableCellInfo* tli);
    virtual void genCellNo(TableCellInfo* tli);
    virtual void genCellIndex(TableCellInfo* tli);
    virtual void genCellId(TableCellInfo* tli);
    virtual void genCellName(TableCellInfo* tli);
    virtual void genCellStart(TableCellInfo* tli);
    virtual void genCellEnd(TableCellInfo* tli);
    virtual void genCellMinStart(TableCellInfo* tli);
    virtual void genCellMaxStart(TableCellInfo* tli);
    virtual void genCellMinEnd(TableCellInfo* tli);
    virtual void genCellMaxEnd(TableCellInfo* tli);
    virtual void genCellStartBuffer(TableCellInfo* tli);
    virtual void genCellEndBuffer(TableCellInfo* tli);
    virtual void genCellStartBufferEnd(TableCellInfo* tli);
    virtual void genCellEndBufferStart(TableCellInfo* tli);
    virtual void genCellDuration(TableCellInfo* tli);
    virtual void genCellEffort(TableCellInfo* tli);
    virtual void genCellProjectId(TableCellInfo* tli);
    virtual void genCellResources(TableCellInfo* tli);
    virtual void genCellResponsible(TableCellInfo* tli);
    virtual void genCellNote(TableCellInfo* tli);
    virtual void genCellStatusNote(TableCellInfo* tli);
    virtual void genCellCost(TableCellInfo* tli);
    virtual void genCellRevenue(TableCellInfo* tli);
    virtual void genCellProfit(TableCellInfo* tli);
    virtual void genCellPriority(TableCellInfo* tli);
    virtual void genCellFlags(TableCellInfo* tli);
    virtual void genCellCompleted(TableCellInfo* tli);
    virtual void genCellStatus(TableCellInfo* tli);
    virtual void genCellReference(TableCellInfo* tli);
    virtual void genCellScenario(TableCellInfo* tli);
    virtual void genCellDepends(TableCellInfo* tli);
    virtual void genCellFollows(TableCellInfo* tli);
    virtual void genCellDailyTask(TableCellInfo* tli);
    virtual void genCellDailyResource(TableCellInfo* tli);
    virtual void genCellDailyAccount(TableCellInfo* tli);
    virtual void genCellWeeklyTask(TableCellInfo* tli);
    virtual void genCellWeeklyResource(TableCellInfo* tli);
    virtual void genCellWeeklyAccount(TableCellInfo* tli);
    virtual void genCellMonthlyTask(TableCellInfo* tli);
    virtual void genCellMonthlyResource(TableCellInfo* tli);
    virtual void genCellMonthlyAccount(TableCellInfo* tli);
    virtual void genCellQuarterlyTask(TableCellInfo* tli);
    virtual void genCellQuarterlyResource(TableCellInfo* tli);
    virtual void genCellQuarterlyAccount(TableCellInfo* tli);
    virtual void genCellYearlyTask(TableCellInfo* tli);
    virtual void genCellYearlyResource(TableCellInfo* tli);
    virtual void genCellYearlyAccount(TableCellInfo* tli);
    virtual void genCellResponsibilities(TableCellInfo* tli);
    virtual void genCellSchedule(TableCellInfo* tli);
    virtual void genCellMinEffort(TableCellInfo* tli);
    virtual void genCellMaxEffort(TableCellInfo* tli);
    virtual void genCellRate(TableCellInfo* tli);
    virtual void genCellKotrusId(TableCellInfo* tli);
    virtual void genCellTotal(TableCellInfo* tli);

    virtual void genCellSummary(TableCellInfo* tli);

protected:
    HTMLReportElement() { }

    QString generateUrl(const QString& key, const QString& txt);
    void generateRightIndented(TableLineInfo* tli, const QString str);

    MacroTable mt;

    BarLabelText barLabels;

    QString rawHead;
    QString rawTail;
} ;

#endif
