/*
 * ReportHtml.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_Html_h_
#define _Report_Html_h_

#include <stdio.h>
#include <time.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qtextstream.h>
#include <qmap.h>

#include "Report.h"
#include "HTMLPrimitives.h"
#include "MacroTable.h"
#include "taskjuggler.h"

class Project;
class ExpressionTree;

/**
 * @short Stores all information about an HTML report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ReportHtml : public Report, public HTMLPrimitives
{
public:
    ReportHtml(Project* p, const QString& f, const QString& df, int dl);
    virtual ~ReportHtml() { }

    enum BarLabelText { BLT_EMPTY = 0, BLT_LOAD };

    void generatePlanTask(const Task* t, const Resource* r, uint no);
    void generateActualTask(const Task* t, const Resource* r);

    void generatePlanResource(const Resource* r, const Task* t, uint no);
    void generateActualResource(const Resource* r, const Task* t);

    void reportHTMLHeader();
    void reportHTMLFooter();

    bool generateTableHeader();

    void generateDepends(const Task* t, bool light);
    void generateFollows(const Task* t, bool light);
    void generateResponsibilities(const Resource* r, bool light);
    void htmlDailyHeaderDays(bool highlightNow = TRUE);
    void htmlDailyHeaderMonths();
    void htmlWeeklyHeaderWeeks(bool highlightNow = TRUE);
    void htmlWeeklyHeaderMonths();
    void htmlMonthlyHeaderMonths(bool highlightNow = TRUE);
    void htmlMonthlyHeaderYears();
    void htmlQuarterlyHeaderQuarters(bool highlightNow = TRUE);
    void htmlQuarterlyHeaderYears();
    void htmlYearHeader();

    void emptyPlan(bool light);
    void emptyActual(bool light);

    void textOneRow(const QString& text, bool light, const QString& align);
    void textTwoRows(const QString& text, bool light, const QString& align);

    void dailyResourcePlan(const Resource* r, const Task* t);
    void dailyResourceActual(const Resource* r, const Task* t);
    void dailyTaskPlan(const Task* t, const Resource* r);
    void dailyTaskActual(const Task* t, const Resource* r);

    void weeklyResourcePlan(const Resource* r, const Task* t);
    void weeklyResourceActual(const Resource* r, const Task* t);
    void weeklyTaskPlan(const Task* t, const Resource* r);
    void weeklyTaskActual(const Task* t, const Resource* r);

    void monthlyResourcePlan(const Resource* r, const Task* t);
    void monthlyResourceActual(const Resource* r, const Task* t);
    void monthlyTaskPlan(const Task* t, const Resource* r);
    void monthlyTaskActual(const Task* t, const Resource* r);

    void taskName(const Task* t, const Resource* r, bool big);
    void resourceName(const Resource* t, const Task* t, bool big);

    void taskCostRev(const Task* t, const Resource* r, double val);
    void resourceCostRev(const Task* t, const Resource* r, double val);

    void taskLoadValue(const Task* t, const Resource* r, double val);
    void resourceLoadValue(const Task* t, const Resource* r, double val);
    
    void scenarioResources(int sc, const Task* t, bool light);

    void reportLoad(double load, const QString& bgcol, bool bold,
                    bool milestone = FALSE);
    void reportPIDs(const QString& pids, const QString bgCol, bool bold);

    void generateSchedule(int sc, const Resource* r, const Task* t);

    void flagList(const CoreAttributes* c1, const CoreAttributes* c2);

    void generateTaskStatus(TaskStatus status, bool light);

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

    void setRawStyleSheet(const QString& styleSheet)
    {
        rawStyleSheet = styleSheet;
    }
    

protected:
    ReportHtml() { }

    QString generateUrl(const QString& key, const QString& txt);

    MacroTable mt;

    uint colDefault;
    uint colDefaultLight;
    uint colWeekend;
    uint colVacation;
    uint colAvailable;
    uint colBooked;
    uint colBookedLight;
    uint colHeader;
    uint colMilestone;
    uint colCompleted;
    uint colCompletedLight;
    uint colToday;

    BarLabelText barLabels;

    QString rawHead;
    QString rawTail;
    QString rawStyleSheet;

    QMap<QString, QString> urls;
} ;

#endif
