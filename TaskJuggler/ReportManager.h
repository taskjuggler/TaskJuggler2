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
class QListViewItem;
class KListView;
class Report;
class CoreAttributes;

class ReportManager : public QObject
{
    Q_OBJECT
public:
    ReportManager(QWidgetStack* v, KListView* b);
    virtual ~ReportManager() { }

    void updateReportList(QPtrListIterator<Report> rli);

    ManagedReportInfo* getCurrentReport() const;

    QListViewItem* getFirstInteractiveReportItem() const;

    QWidgetStack* getReportStack() const { return reportStack; }

    void setFocusToReport() const;

    bool showReport(QListViewItem*);

    bool isProjectLoaded() const;

    void setLoadingProject(bool lp);

    void clear();

signals:
    void signalChangeStatusBar(const QString& text);
    void signalEditCoreAttributes(CoreAttributes* ca);

public slots:
    void zoomIn();
    void zoomOut();
    void closeCurrentReport();
    void changeStatusBar(const QString& text);
    void editCoreAttributes(CoreAttributes* ca);

private:
    ReportManager() { }

    void updateReportBrowser();

    QWidgetStack* reportStack;
    KListView* browser;

    KListViewItem* qtReports;
    KListViewItem* htmlReports;
    KListViewItem* csvReports;
    KListViewItem* xmlReports;
    KListViewItem* exportReports;

    QPtrList<ManagedReportInfo> reports;

    bool loadingProject;
} ;

#endif

