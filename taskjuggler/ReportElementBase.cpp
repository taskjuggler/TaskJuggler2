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
#include "tjlib-internal.h"
#include "Project.h"

ReportElementBase::ReportElementBase(Report* r) :
    report(r),
    loadUnit(shortAuto),
    numberFormat(),
    currencyFormat()
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


