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

#include "TjHTMLReport.h"

#include <qlayout.h>

#include <klocale.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <krun.h>

#include <HTMLReport.h>

TjHTMLReport::TjHTMLReport(QWidget* p, ReportManager* m, Report* rDef,
                                 const QString& n) :
    TjUIReportBase(p, m, rDef, n)
{
    QVBoxLayout* hl = new QVBoxLayout(this, 0);
    hl->setAutoAdd(true);
    browser = new KHTMLPart(this);

    /* It is very unlikely that we need these advanced KHTML features, so
     * let's disable them for security reasons. */
    browser->setJScriptEnabled(false);
    browser->setJavaEnabled(false);
    browser->setMetaRefreshEnabled(false);
    browser->setPluginsEnabled(false);
    browser->setOnlyLocalReferences(true);

    connect(browser->browserExtension(),
            SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs&)),
            this, SLOT(openURLRequest(const KURL &, const KParts::URLArgs&)));
    connect(browser, SIGNAL(onURL(const QString&)),
            this, SLOT(showURLinStatusBar(const QString&)));
}

bool
TjHTMLReport::generateReport()
{
    HTMLReport* htmlReport = dynamic_cast<HTMLReport*>(report);
    if (!htmlReport->generate())
        return false;
    KURL reportURL = KURL::fromPathOrURL(report->getFullFileName());
    browser->openURL(reportURL);

    return true;
}

void
TjHTMLReport::print()
{
    browser->view()->print();
}

void
TjHTMLReport::show()
{
    QWidget::show();
}

void
TjHTMLReport::hide()
{
    QWidget::hide();
}

void
TjHTMLReport::showURLinStatusBar(const QString& url)
{
    emit signalChangeStatusBar(url);
}

void
TjHTMLReport::openURLRequest(const KURL &url, const KParts::URLArgs&)
{
    KRun::runURL(url, "text/html");
}

#include "TjHTMLReport.moc"

