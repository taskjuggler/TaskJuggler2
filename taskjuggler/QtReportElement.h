/*
 * QtReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtReportElement_h_
#define _QtReportElement_h_

#include "ReportElement.h"

class QColor;
class QString;

class Project;
class Report;
class ExpressionTree;
class TableLineInfo;
class TableCellInfo;
class Interval;

/**
 * @short Models the basic component of an Qt report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class QtReportElement : public ReportElement
{
public:
    QtReportElement(Report* r, const QString& df, int dl);
    virtual ~QtReportElement() { }

    virtual bool generate()  { return false; }

    virtual void genHeadDefault(TableCellInfo*) { }
    virtual void genHeadCurrency(TableCellInfo*) { }
    virtual void genHeadDaily1(TableCellInfo*) { }
    virtual void genHeadDaily2(TableCellInfo*) { }
    virtual void genHeadWeekly1(TableCellInfo*) { }
    virtual void genHeadWeekly2(TableCellInfo*) { }
    virtual void genHeadMonthly1(TableCellInfo*) { }
    virtual void genHeadMonthly2(TableCellInfo*) { }
    virtual void genHeadQuarterly1(TableCellInfo*) { }
    virtual void genHeadQuarterly2(TableCellInfo*) { }
    virtual void genHeadYear(TableCellInfo*) { }

    virtual void genCellEmpty(TableCellInfo*) { }
    virtual void genCellSequenceNo(TableCellInfo*) { }
    virtual void genCellNo(TableCellInfo*) { }
    virtual void genCellHierarchNo(TableCellInfo*) { }
    virtual void genCellIndex(TableCellInfo*) { }
    virtual void genCellHierarchIndex(TableCellInfo*) { }
    virtual void genCellId(TableCellInfo*) { }
    virtual void genCellName(TableCellInfo*) { }
    virtual void genCellStart(TableCellInfo*) { }
    virtual void genCellEnd(TableCellInfo*) { }
    virtual void genCellMinStart(TableCellInfo*) { }
    virtual void genCellMaxStart(TableCellInfo*) { }
    virtual void genCellMinEnd(TableCellInfo*) { }
    virtual void genCellMaxEnd(TableCellInfo*) { }
    virtual void genCellStartBuffer(TableCellInfo*) { }
    virtual void genCellEndBuffer(TableCellInfo*) { }
    virtual void genCellStartBufferEnd(TableCellInfo*) { }
    virtual void genCellEndBufferStart(TableCellInfo*) { }
    virtual void genCellDuration(TableCellInfo*) { }
    virtual void genCellEffort(TableCellInfo*) { }
    virtual void genCellFreeLoad(TableCellInfo*) { }
    virtual void genCellUtilization(TableCellInfo*) { }
    virtual void genCellCriticalness(TableCellInfo*) { }
    virtual void genCellPathCriticalness(TableCellInfo*) { }
    virtual void genCellProjectId(TableCellInfo*) { }
    virtual void genCellProjectIDs(TableCellInfo*) { }
    virtual void genCellResources(TableCellInfo*) { }
    virtual void genCellResponsible(TableCellInfo*) { }
    virtual void genCellText(TableCellInfo*) { }
    virtual void genCellStatusNote(TableCellInfo*) { }
    virtual void genCellCost(TableCellInfo*) { }
    virtual void genCellRevenue(TableCellInfo*) { }
    virtual void genCellProfit(TableCellInfo*) { }
    virtual void genCellPriority(TableCellInfo*) { }
    virtual void genCellFlags(TableCellInfo*) { }
    virtual void genCellCompleted(TableCellInfo*) { }
    virtual void genCellStatus(TableCellInfo*) { }
    virtual void genCellReference(TableCellInfo*) { }
    virtual void genCellScenario(TableCellInfo*) { }
    virtual void genCellDepends(TableCellInfo*) { }
    virtual void genCellFollows(TableCellInfo*) { }
    virtual void genCellDailyTask(TableCellInfo*) { }
    virtual void genCellDailyResource(TableCellInfo*) { }
    virtual void genCellDailyAccount(TableCellInfo*) { }
    virtual void genCellWeeklyTask(TableCellInfo*) { }
    virtual void genCellWeeklyResource(TableCellInfo*) { }
    virtual void genCellWeeklyAccount(TableCellInfo*) { }
    virtual void genCellMonthlyTask(TableCellInfo*) { }
    virtual void genCellMonthlyResource(TableCellInfo*) { }
    virtual void genCellMonthlyAccount(TableCellInfo*) { }
    virtual void genCellQuarterlyTask(TableCellInfo*) { }
    virtual void genCellQuarterlyResource(TableCellInfo*) { }
    virtual void genCellQuarterlyAccount(TableCellInfo*) { }
    virtual void genCellYearlyTask(TableCellInfo*) { }
    virtual void genCellYearlyResource(TableCellInfo*) { }
    virtual void genCellYearlyAccount(TableCellInfo*) { }
    virtual void genCellResponsibilities(TableCellInfo*) { }
    virtual void genCellSchedule(TableCellInfo*) { }
    virtual void genCellMinEffort(TableCellInfo*) { }
    virtual void genCellMaxEffort(TableCellInfo*) { }
    virtual void genCellEfficiency(TableCellInfo*) { }
    virtual void genCellRate(TableCellInfo*) { }
    virtual void genCellKotrusId(TableCellInfo*) { }
    virtual void genCellTotal(TableCellInfo*) { }
    virtual void genCellSummary(TableCellInfo*) { }
} ;

#endif
