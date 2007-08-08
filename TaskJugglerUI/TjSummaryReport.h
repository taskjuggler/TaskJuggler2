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

#ifndef _TjSummaryReport_h_
#define _TjSummaryReport_h_

#include "TjUIReportBase.h"

class Project;
class Report;
class QTextBrowser;

class TjSummaryReport : public TjUIReportBase
{
    Q_OBJECT

public:
    TjSummaryReport(QWidget* p, ReportManager* m, const Project* pr,
                 const QString& n = QString::null);
    virtual ~TjSummaryReport() { }

    virtual void setFocus() { }

    virtual bool generateReport();

    virtual void print();

public slots:
    virtual void show();
    virtual void hide();

private:
    const Project* project;
    QTextBrowser* textBrowser;
} ;

#endif

