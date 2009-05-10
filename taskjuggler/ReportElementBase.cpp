/*
 * ReportElementBase.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: ReportElement.h 1335 2006-09-24 13:49:05Z cs $
 */

#include "ReportElementBase.h"
#include "TableLineInfo.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Account.h"
#include "Task.h"
#include "Resource.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"

ReportElementBase::ReportElementBase(Report* r, const QString& df, int dl) :
    report(r),
    loadUnit(shortAuto),
    numberFormat(),
    currencyFormat(),
    mt(),
    defFileName(df),
    defFileLine(dl)
{
}

bool
ReportElementBase::setLoadUnit(const QString& u)
{
    if (u == KW("minutes"))
        loadUnit = minutes;
    else if (u == KW("hours"))
        loadUnit = hours;
    else if (u == KW("days"))
        loadUnit = days;
    else if (u == KW("weeks"))
        loadUnit = weeks;
    else if (u == KW("months"))
        loadUnit = months;
    else if (u == KW("years"))
        loadUnit = years;
    else if (u == KW("shortauto"))
        loadUnit = shortAuto;
    else if (u == KW("longauto"))
        loadUnit = longAuto;
    else
        return false;

    return true;
}

QString
ReportElementBase::scaledDuration(double t, const RealFormat& realFormat,
                                  bool showUnit, bool longUnit) const
{
    QValueList<double> factors;

    factors.append(24 * 60);
    factors.append(24);
    factors.append(1);
    factors.append(1.0 / 7);
    factors.append(1.0 / 30.42);
    factors.append(1.0 / 365);

    return scaledValue(t, realFormat, showUnit, longUnit, factors);
}

QString
ReportElementBase::scaledLoad(double t, const RealFormat& realFormat,
                              bool showUnit, bool longUnit) const
{
    QValueList<double> factors;

    factors.append(report->getProject()->getDailyWorkingHours() * 60);
    factors.append(report->getProject()->getDailyWorkingHours());
    factors.append(1);
    factors.append(1.0 / report->getProject()->getWeeklyWorkingDays());
    factors.append(1.0 / report->getProject()->getMonthlyWorkingDays());
    factors.append(1.0 / report->getProject()->getYearlyWorkingDays());

    return scaledValue(t, realFormat, showUnit, longUnit, factors);
}

QString
ReportElementBase::scaledValue(double t, const RealFormat& realFormat,
                               bool showUnit, bool longUnit,
                               const QValueList<double>& factors) const
{
    QStringList variations;
    const char* shortUnit[] = { "min", "h", "d", "w", "m", "y" };
    const char* unit[] = { "minute", "hour", "day", "week", "month", "year" };
    const char* units[] = { "minutes", "hours", "days", "weeks", "months",
        "years"};
    double max[] = { 60, 48, 0, 8, 24, 0 };

    QString str;

    if (loadUnit == shortAuto || loadUnit == longAuto)
    {
        for (QValueList<double>::ConstIterator it = factors.begin();
             it != factors.end(); ++it)
        {
            str = realFormat.format(t * *it, false);
            int idx = factors.findIndex(*it);
            if ((*it != 1.0 && str == "0") ||
                (max[idx] != 0 && max[idx] < (t * *it)))
                variations.append("");
            else
                variations.append(str);
        }

        uint shortest = 2;      // default to days in case all are the same
        for (QStringList::Iterator it = variations.begin();
             it != variations.end();
             ++it)
        {
            if ((*it).length() > 0 &&
                (*it).length() < variations[shortest].length())
            {
                shortest = variations.findIndex(*it);
            }
        }
        str = variations[shortest];
        if (loadUnit == longAuto)
        {
            if (variations[shortest] == "1")
                str += QString(" ") + unit[shortest];
            else
                str += QString(" ") + units[shortest];
        }
        else
            str += shortUnit[shortest];
    }
    else
    {
        switch (loadUnit)
        {
            case minutes:
                str = realFormat.format(t * factors[0], false);
                break;
            case hours:
                str = realFormat.format(t * factors[1], false);
                break;
            case days:
                str = realFormat.format(t * factors[2], false);
                break;
            case weeks:
                str = realFormat.format(t * factors[3], false);
                break;
            case months:
                str = realFormat.format(t * factors[4], false);
                break;
            case years:
                str = realFormat.format(t * factors[5], false);
                break;
            case shortAuto:
            case longAuto:
                break;  // handled above switch statement already
        }
        // Add unit in case it's forced by caller.
        if (showUnit && loadUnit <= years)
            str += longUnit ? QString(" ") + units[loadUnit] :
                QString(shortUnit[loadUnit]);
    }
    return str;
}

void
ReportElementBase::setMacros(TableLineInfo* tli)
{
    mt.clear();

    /* In some cases it might be useful to have not only the ID of the current
     * property but also the assigned property (e. g. in task reports with
     * resources, we want the task ID while processing the resource line. */
    if (tli->task)
    {
        if (tli->task->getAccount()) mt.addMacro(new Macro(KW("accounts"), tli->task->getAccount()->getName(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("completed"), tli->task->getComplete(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("completedeffort"), tli->task->getCompletedLoad(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("criticalness"), tli->task->getCriticalness(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("duration"), tli->task->getDuration(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("effort"), tli->task->getEffort(tli->sc), defFileName, defFileLine));
        if (tli->task->getEnd(tli->sc))
            mt.addMacro(new Macro(KW("end"), time2user(tli->task->getEnd(tli->sc), tli->task->getProject()->getTimeFormat()), defFileName, defFileLine));
        if (tli->task->getMaxEnd(tli->sc))
            mt.addMacro(new Macro(KW("maxend"), time2user(tli->task->getMaxEnd(tli->sc), tli->task->getProject()->getTimeFormat()), defFileName, defFileLine));
        if (tli->task->getMaxStart(tli->sc))
            mt.addMacro(new Macro(KW("maxstart"), time2user(tli->task->getMaxStart(tli->sc), tli->task->getProject()->getTimeFormat()), defFileName, defFileLine));
        if (tli->task->getMinEnd(tli->sc))
            mt.addMacro(new Macro(KW("minend"), time2user(tli->task->getMinEnd(tli->sc), tli->task->getProject()->getTimeFormat()), defFileName, defFileLine));
        if (tli->task->getMinStart(tli->sc))
            mt.addMacro(new Macro(KW("minstart"), time2user(tli->task->getMinStart(tli->sc), tli->task->getProject()->getTimeFormat()), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("note"), tli->task->getNote(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("pathcriticalness"), tli->task->getPathCriticalness(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("priority"), tli->task->getPriority(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("reference"), tli->task->getReference(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("remainingeffort"), tli->task->getRemainingLoad(tli->sc), defFileName, defFileLine));
        if (tli->task->getResponsible())
            mt.addMacro(new Macro(KW("responsible"), tli->task->getResponsible()->getName(), defFileName, defFileLine));
        if (tli->task->getStart(tli->sc))
            mt.addMacro(new Macro(KW("start"), time2user(tli->task->getStart(tli->sc), tli->task->getProject()->getTimeFormat()), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("status"), tli->task->getStatusText(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("statusnote"), tli->task->getStatusNote(tli->sc), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("taskid"), tli->task->getId(), defFileName, defFileLine));

        QString label = "";
        for (ResourceListIterator rli
                (tli->task->getBookedResourcesIterator(tli->sc)); *rli != 0; ++rli)
        {
            if (!label.isEmpty())
                label += ", ";

            label += (*rli)->getName();
        }
        mt.addMacro(new Macro(KW("resources"), label, defFileName, defFileLine));
    }
    if (tli->resource)
    {
        mt.addMacro(new Macro(KW("efficiency"), tli->resource->getEfficiency(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("mineffort"), tli->resource->getMinEffort(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("rate"), tli->resource->getRate(), defFileName, defFileLine));
        mt.addMacro(new Macro(KW("resourceid"), tli->resource->getId(), defFileName, defFileLine));
    }

    if (tli->account)
    {
        mt.addMacro(new Macro(KW("accountid"), tli->account->getId(), defFileName, defFileLine));
    }
                              
    // Set macros for built-in attributes.
    mt.addMacro(new Macro(KW("id"), tli->ca1 ? tli->ca1->getId() :
                          QString::null,
                          defFileName, defFileLine));
    mt.addMacro(new Macro(KW("no"), tli->ca1 ?
                          QString("%1").arg(tli->ca1->getSequenceNo()) :
                          QString::null,
                          defFileName, defFileLine));
    mt.addMacro(new Macro(KW("index"), tli->ca1 ?
                          QString("%1").arg(tli->ca1->getIndex()) :
                          QString::null,
                          defFileName, defFileLine));
    mt.addMacro(new Macro(KW("hierarchno"), tli->ca1 ?
                          tli->ca1->getHierarchNo() : QString::null,
                          defFileName, defFileLine));
    mt.addMacro(new Macro(KW("hierarchindex"),
                          tli->ca1 ? tli->ca1->getHierarchIndex() :
                          QString::null,
                          defFileName, defFileLine));
    mt.addMacro(new Macro(KW("hierarchlevel"),
                          tli->ca1 ? tli->ca1->getHierarchLevel() :
                          QString::null,
                          defFileName, defFileLine));
    mt.addMacro(new Macro(KW("name"),
                          tli->ca1 ? tli->ca1->getName() : QString::null,
                          defFileName, defFileLine));


    setPropertyMacros(tli, report->getProject()->getTaskAttributeDict());
    setPropertyMacros(tli, report->getProject()->getResourceAttributeDict());
    setPropertyMacros(tli, report->getProject()->getAccountAttributeDict());
}

void
ReportElementBase::setPropertyMacros(TableLineInfo* tli,
   const QDictIterator<CustomAttributeDefinition>& d)
{
    QDictIterator<CustomAttributeDefinition> cadi(d);
    for ( ; cadi.current(); ++cadi)
    {
        const CustomAttribute* custAttr;
        QString macroName = cadi.currentKey();
        QString macroValue;
        if (tli->ca1 &&
            (custAttr = tli->ca1->getCustomAttribute(macroName)) != 0)
        {
            switch (custAttr->getType())
            {
                case CAT_Text:
                    macroValue = static_cast<const TextAttribute*>(custAttr)->getText();
                    break;
                case CAT_Reference:
                    macroValue = static_cast<const ReferenceAttribute*>(custAttr)->getURL();
                    break;
                default:
                    break;
            }
        }
        mt.addMacro(new Macro(macroName, macroValue, defFileName,
                              defFileLine));
    }
}

const QString
ReportElementBase::expandReportVariable(const QString& t) const
{
//     printf ("ReportElementBase::expandReportVariable <%s>", t.latin1());
    QStringList sl("");
    return mt.expandReportVariable(t, &sl);
}

