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

    void generateFirstTask(const Task* t, const Resource* r, uint no);
    void generateNextTask(int sc, const Task* t, const Resource* r);

    void generateFirstResource(const Resource* r, const Task* t, uint no);
    void generateNextResource(int sc, const Resource* r, const Task* t);

    void textOneRow(const QString& text, bool light, const QString& align);
    void textMultiRows(const QString& text, bool light, const QString& align);

    void reportPIDs(const QString& pids, const QString bgCol, bool bold);

    void reportLoad(double load, const QString& bgcol, bool bold,
                    bool milestone = FALSE);

    void setBarLabels(BarLabelText blt) { barLabels = blt; }

    void registerUrl(const QString& key, const QString& url = QString::null)
    {
        urls[key] = url;
    }
    bool setUrl(const QString& key, const QString& url);
    const QString* getUrl(const QString& key) const;

    void setRawHead(const QString& head)
    {
        rawHead = head;
    }

    void setRawTail(const QString& tail)
    {
        rawTail = tail;
    }

    virtual void genHeadDefault(TableColumnFormat* tcf);
    virtual void genHeadCurrency(TableColumnFormat* tcf);
    virtual void genHeadDaily1(TableColumnFormat* tcf);
    virtual void genHeadDaily2(TableColumnFormat* tcf);
    virtual void genHeadWeekly1(TableColumnFormat* tcf);
    virtual void genHeadWeekly2(TableColumnFormat* tcf);
    virtual void genHeadMonthly1(TableColumnFormat* tcf);
    virtual void genHeadMonthly2(TableColumnFormat* tcf);
    virtual void genHeadQuarterly1(TableColumnFormat* tcf);
    virtual void genHeadQuarterly2(TableColumnFormat* tcf);
    virtual void genHeadYear(TableColumnFormat* tcf);
   
    virtual void genCellEmpty(TableLineInfo*); 
    virtual void genCellSequenceNo(TableLineInfo* tli);
    virtual void genCellNo(TableLineInfo* tli);
    virtual void genCellIndex(TableLineInfo* tli);
    virtual void genCellId(TableLineInfo* tli);
    virtual void genCellName(TableLineInfo* tli);
    virtual void genCellStart(TableLineInfo* tli);
    virtual void genCellEnd(TableLineInfo* tli);
    virtual void genCellMinStart(TableLineInfo* tli);
    virtual void genCellMaxStart(TableLineInfo* tli);
    virtual void genCellMinEnd(TableLineInfo* tli);
    virtual void genCellMaxEnd(TableLineInfo* tli);
    virtual void genCellStartBuffer(TableLineInfo* tli);
    virtual void genCellEndBuffer(TableLineInfo* tli);
    virtual void genCellStartBufferEnd(TableLineInfo* tli);
    virtual void genCellEndBufferStart(TableLineInfo* tli);
    virtual void genCellDuration(TableLineInfo* tli);
    virtual void genCellEffort(TableLineInfo* tli);
    virtual void genCellProjectId(TableLineInfo* tli);
    virtual void genCellResources(TableLineInfo* tli);
    virtual void genCellResponsible(TableLineInfo* tli);
    virtual void genCellNote(TableLineInfo* tli);
    virtual void genCellStatusNote(TableLineInfo* tli);
    virtual void genCellCost(TableLineInfo* tli);
    virtual void genCellRevenue(TableLineInfo* tli);
    virtual void genCellProfit(TableLineInfo* tli);
    virtual void genCellPriority(TableLineInfo* tli);
    virtual void genCellFlags(TableLineInfo* tli);
    virtual void genCellCompleted(TableLineInfo* tli);
    virtual void genCellStatus(TableLineInfo* tli);
    virtual void genCellReference(TableLineInfo* tli);
    virtual void genCellScenario(TableLineInfo* tli);
    virtual void genCellDepends(TableLineInfo* tli);
    virtual void genCellFollows(TableLineInfo* tli);
    virtual void genCellDailyTask(TableLineInfo* tli);
    virtual void genCellDailyResource(TableLineInfo* tli);
    virtual void genCellWeeklyTask(TableLineInfo* tli);
    virtual void genCellWeeklyResource(TableLineInfo* tli);
    virtual void genCellMonthlyTask(TableLineInfo* tli);
    virtual void genCellMonthlyResource(TableLineInfo* tli);
    virtual void genCellResponsibilities(TableLineInfo* tli);
    virtual void genCellSchedule(TableLineInfo* tli);
    virtual void genCellMinEffort(TableLineInfo* tli);
    virtual void genCellMaxEffort(TableLineInfo* tli);
    virtual void genCellRate(TableLineInfo* tli);
    virtual void genCellKotrusId(TableLineInfo* tli);

protected:
    HTMLReportElement() { }

    QString generateUrl(const QString& key, const QString& txt);
    void generateRightIndented(TableLineInfo* tli, const QString str);

    MacroTable mt;

    BarLabelText barLabels;

    QString rawHead;
    QString rawTail;

    QMap<QString, QString> urls;
} ;

#endif
