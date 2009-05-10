/*
 * ReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: ReportElement.h 1335 2006-09-24 13:49:05Z cs $
 */

#ifndef _ReportElementBase_h_
#define _ReportElementBase_h_

#include "Report.h"
#include "MacroTable.h"

class TableLineInfo;

/**
 * @short A class that forms the base for elements of a report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class ReportElementBase {
public:
    ReportElementBase(Report* r, const QString& df = "", int dl = 0);
    virtual ~ReportElementBase() { }

    QString scaledDuration(double t, const RealFormat& realFormat,
                           bool showUnit = false, bool longUnit = false) const;
    QString scaledLoad(double t, const RealFormat& realFormat,
                       bool showUnit = false, bool longUnit = false) const;

    bool setLoadUnit(const QString& u);

    const RealFormat& getNumberFormat() const { return numberFormat; }

    void setMacros(TableLineInfo* tli);

    void setPropertyMacros(TableLineInfo* tli,
                           const QDictIterator<CustomAttributeDefinition>& d);

    const QString expandReportVariable(const QString& t) const;

protected:
    QString scaledValue(double t, const RealFormat& realFormat,
                        bool showUnit, bool longUnit,
                        const QValueList<double>& factors) const;

    Report* report;

    LoadUnit loadUnit;

    RealFormat numberFormat;
    RealFormat currencyFormat;

    MacroTable mt;

    /* We store the location of the report definition in case we need it
     * for error reporting. */
    QString defFileName;
    int defFileLine;


};

#endif

