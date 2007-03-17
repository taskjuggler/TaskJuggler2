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

#ifndef _TjHTMLReport_h_
#define _TjHTMLReport_h_

#include "TjUIReportBase.h"

#include <kparts/browserrun.h>

class KURL;
class KHTMLPart;
class Project;
class Report;

class TjHTMLReport : public TjUIReportBase
{
    Q_OBJECT

public:
    TjHTMLReport(QWidget* p, ReportManager* m, Report* rDef,
                 const QString& n = QString::null);
    virtual ~TjHTMLReport() { }

    virtual bool generateReport();

    virtual void print();

public slots:
    virtual void show();
    virtual void hide();

    void showURLinStatusBar(const QString& url);
    void openURLRequest(const KURL &url, const KParts::URLArgs&);

private:
    KHTMLPart* browser;
} ;

#endif

