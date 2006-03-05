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

#ifndef _ReportManager_h_
#define _ReportManager_h_

#include <list>

#include <qobject.h>
#include <qptrlist.h>

#include "ManagedReportInfo.h"

class QWidgetStack;
class QString;
class QListViewItem;
class KMainWindow;
class KSelectAction;
class KListView;
class KListViewSearchLine;
class KURL;
class Project;
class Report;
class CoreAttributes;
class Report;

class ReportManager : public QObject
{
    Q_OBJECT
public:
    ReportManager(KMainWindow* mainWindow, QWidgetStack* v, KListView* b,
                  KListViewSearchLine* s);
    virtual ~ReportManager();

    void updateReportList(const Project* pr);

    ManagedReportInfo* getCurrentReport() const;

    QListViewItem* getFirstInteractiveReportItem() const;

    QWidgetStack* getReportStack() const { return reportStack; }

    void setFocusToReport() const;

    void enableReportActions(bool enable);

    bool generateReport(QListViewItem*);
    bool showReport(QListViewItem*, bool& showReport);
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
    void signalEditFile(const KURL& url);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomTo();
    void updateZoomSelector(const QStringList& items, unsigned int current);
    void closeCurrentReport();
    void changeStatusBar(const QString& text);
    void editCoreAttributes(CoreAttributes* ca);
    void editReport(const Report* report);

private:
    ReportManager() { }

    void updateReportBrowser();

    KMainWindow* mainWindow;
    KSelectAction* zoomSelector;

    const Project* project;

    QWidgetStack* reportStack;
    KListView* browser;
    KListViewSearchLine* searchLine;

    KListViewItem* qtReports;
    KListViewItem* htmlReports;
    KListViewItem* csvReports;
    KListViewItem* xmlReports;
    KListViewItem* icalReports;
    KListViewItem* exportReports;

    std::list<ManagedReportInfo*> reports;

    bool loadingProject;
} ;

#endif

