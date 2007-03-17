/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjUIReportBase.h 1275 2006-03-05 19:16:55Z cs $
 */

#ifndef _TjUIReportBase_h_
#define _TjUIReportBase_h_

#include <qwidget.h>
#include "ReportElementBase.h"

class CoreAttributes;
class Report;
class ReportManager;

class TjUIReportBase : public QWidget, public ReportElementBase
{
    Q_OBJECT
public:
    TjUIReportBase(QWidget* p, ReportManager* m, Report* rDef,
                 const QString& n = QString::null);
    virtual ~TjUIReportBase() { }

    Report* getReportDefinition() const { return report; }

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
    ReportManager* manager;

    bool loadingProject;
} ;

#endif

