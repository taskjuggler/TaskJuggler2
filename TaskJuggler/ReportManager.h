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

#ifndef _ReportManager_h_
#define _ReportManager_h_

#include <qobject.h>
#include <qptrlist.h>

#include "ManagedReportInfo.h"

class QWidgetStack;
class QString;
class KListView;
class KListViewItem;
class Report;

class ReportManager : public QObject
{
    Q_OBJECT
public:
    ReportManager(QWidgetStack* v, KListView* b);
    virtual ~ReportManager() { }

    void updateReportList(QPtrListIterator<Report> rli);

    ManagedReportInfo* getCurrentReport() const;

    QWidgetStack* getViewStack() const { return viewStack; }

    void setFocusToReport() const;

    bool isProjectLoaded() const;

    void clear();

public slots:
    void showReport(KListViewItem*);
    void closeCurrentReport();

private:
    ReportManager() { }

    void updateReportBrowser();

    QWidgetStack* viewStack;
    KListView* browser;

    KListViewItem* qtReports;
    KListViewItem* htmlReports;
    KListViewItem* csvReports;
    KListViewItem* xmlReports;

    QPtrList<ManagedReportInfo> reports;
} ;

#endif

