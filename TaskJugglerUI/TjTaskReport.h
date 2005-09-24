/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjTaskReport_h_
#define _TjTaskReport_h_

#include "TjReport.h"

class QPoint;
class QListViewItem;
class KPrinter;
class CoreAttributes;
class QtTaskReportElement;
class TjPrintReport;

class TjTaskReport : public TjReport
{
public:
    TjTaskReport(QWidget* p, Report* const rDef,
                 const QString& n = QString::null);
    virtual ~TjTaskReport();

    virtual const QtReportElement* getReportElement() const;

    virtual TjPrintReport* newPrintReport(QPaintDevice* pd);

protected:
    virtual bool generateList();

    virtual QString generateStatusBarText(const QPoint& pos,
                                          const CoreAttributes* ca,
                                          const CoreAttributes* parent) const;

private:
    QListViewItem* getTaskListEntry(const Task* t);

    QtTaskReportElement* reportElement;
} ;

#endif
