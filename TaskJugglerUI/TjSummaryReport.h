/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjReport.h 1181 2005-10-24 08:29:54Z cs $
 */

#ifndef _TjSummaryReport_h_
#define _TjSummaryReport_h_

#include "TjReportBase.h"

class Project;
class Report;
class QTextBrowser;

class TjSummaryReport : public TjReportBase
{
    Q_OBJECT

public:
    TjSummaryReport(QWidget* p, const Project* pr,
                 const QString& n = QString::null);
    virtual ~TjSummaryReport() { }

    virtual bool generateReport();

    virtual void print();

public slots:
    virtual void show();
    virtual void hide();

protected:
    TjSummaryReport() { }

private:
    const Project* project;
    QTextBrowser* textBrowser;
} ;

#endif

