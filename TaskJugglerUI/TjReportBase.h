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

#ifndef _TjReportBase_h_
#define _TjReportBase_h_

#include <qwidget.h>

class CoreAttributes;
class Report;
class ReportManager;

class TjReportBase : public QWidget
{
    Q_OBJECT
public:
    TjReportBase(QWidget* p, ReportManager* m, Report* const rDef,
                 const QString& n = QString::null);
    virtual ~TjReportBase() { }

    Report* getReportDefinition() const { return reportDef; }

    virtual bool generateReport() = 0;

    void setLoadingProject(bool lp) { loadingProject = lp; }

    virtual void print();

signals:
    void signalChangeStatusBar(const QString& text);
    void signalEditCoreAttributes(CoreAttributes*);

public slots:
    virtual void zoomTo(const QString& label);
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void show();
    virtual void hide();

protected:
    TjReportBase() : reportDef(0) { }

    ReportManager* manager;
    Report* const reportDef;

    bool loadingProject;
} ;

#endif

