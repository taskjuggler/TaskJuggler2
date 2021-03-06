/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjResourceReport_h_
#define _TjResourceReport_h_

#include "TjReport.h"

class QPoint;
class KPrinter;
class CoreAttributes;
class QtResourceReportElement;
class TjPrintReport;

class TjResourceReport : public TjReport
{
public:
    TjResourceReport(QWidget* p, ReportManager* m, Report* const rDef,
                     const QString& n = QString::null);
    virtual ~TjResourceReport();

    virtual QtReportElement* getReportElement() const;

    virtual TjPrintReport* newPrintReport(KPrinter* pr);

protected:
    virtual bool generateList();

    virtual QString generateStatusBarText(const QPoint& pos,
                                          CoreAttributes* ca,
                                          CoreAttributes* parent);

private:
    QtResourceReportElement* reportElement;
} ;

#endif

