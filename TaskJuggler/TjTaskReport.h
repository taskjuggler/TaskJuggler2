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
class CoreAttributes;
class QListViewItem;
class QtTaskReportElement;

class TjTaskReport : public TjReport
{
public:
    TjTaskReport(QWidget* p, Report* const rDef,
                 const QString& n = QString::null);
    virtual ~TjTaskReport();

protected:
    virtual bool generateList();
    virtual bool generateChart(bool autoFit);

    virtual QString generateStatusBarText(const QPoint& pos,
                                          const CoreAttributes* ca) const;

private:
    void generateGanttTasks();
    void drawTask(Task* const t, int y);
    void drawDependencies(Task* const t1, QListViewItem* t1lvi);
    void drawTaskResources(Task* const t);
    void drawResourceLoadColum(Task* const t, Resource* const r,
                               time_t start, time_t end, int rY);
    QListViewItem* getTaskListEntry(const Task* t);

    QtTaskReportElement* reportElement;
} ;

#endif

