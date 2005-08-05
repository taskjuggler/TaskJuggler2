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
    TjResourceReport(QWidget* p, Report* const rDef,
                     const QString& n = QString::null);
    virtual ~TjResourceReport();

    virtual const QtReportElement* getReportElement() const;

    virtual TjPrintReport* newPrintReport(QPaintDevice* pd);

protected:
    virtual bool generateList();
    virtual bool generateChart(bool autoFit);

    virtual QString generateStatusBarText(const QPoint& pos,
                                          const CoreAttributes* ca,
                                          const CoreAttributes* parent) const;

private:
    bool generateChartLoadBars();
    void drawResource(const Resource* r, int y);
    bool drawResourceTasks(const Resource* r);
    void drawResourceLoadColumn(const Resource* r, time_t start,
                                time_t end, int rY);
    void drawTaskLoadColumn(const Task* t, const Resource* r,
                            time_t start, time_t end, int rY);
    void drawTaskOutline(const Task* t, int y);
    QListViewItem* getResourceListEntry(const Resource* r);
    QListViewItem* getTaskListEntry(const Task* t, const Resource* r);

    QtResourceReportElement* reportElement;
} ;

#endif

