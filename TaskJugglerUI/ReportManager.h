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
class KListViewSearchLine;
class KPrinter;
class Report;
class CoreAttributes;
class Report;

class ReportManager : public QObject
{
    Q_OBJECT
public:
    ReportManager(QWidgetStack* v, KListView* b, KListViewSearchLine* s);
    virtual ~ReportManager();

    void updateReportList(QPtrListIterator<Report> rli);

    ManagedReportInfo* getCurrentReport() const;

    QListViewItem* getFirstInteractiveReportItem() const;

    QWidgetStack* getReportStack() const { return reportStack; }

    void setFocusToReport() const;

    bool generateReport(QListViewItem*);
    bool showReport(QListViewItem*);
    void showRMBMenu(QListViewItem*, const QPoint&, int, bool& errors,
                     bool& showReport);

    bool isProjectLoaded() const;

    void setLoadingProject(bool lp);

    void clear();

    void print();

signals:
    void signalChangeStatusBar(const QString& text);
    void signalEditCoreAttributes(CoreAttributes* ca);
    void signalEditReport(const Report* report);

public slots:
    void zoomIn();
    void zoomOut();
    void closeCurrentReport();
    void changeStatusBar(const QString& text);
    void editCoreAttributes(CoreAttributes* ca);
    void editReport(const Report* report);

private:
    ReportManager() { }

    void updateReportBrowser();

    QWidgetStack* reportStack;
    KListView* browser;
    KListViewSearchLine* searchLine;

    KListViewItem* qtReports;
    KListViewItem* htmlReports;
    KListViewItem* csvReports;
    KListViewItem* xmlReports;
    KListViewItem* icalReports;
    KListViewItem* exportReports;

    QPtrList<ManagedReportInfo> reports;
    KPrinter* printer;

    bool loadingProject;
} ;

#endif

