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

#include "ReportManager.h"

#include <qwidgetstack.h>
#include <qstring.h>

#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kurl.h>
#include <krun.h>

#include "Report.h"
#include "HTMLReport.h"

#include "ManagedReportInfo.h"
#include "TjReport.h"
#include "TjTaskReport.h"
#include "TjResourceReport.h"

ReportManager::ReportManager(QWidgetStack* v, KListView* b) :
    reportStack(v), browser(b)
{
    reports.setAutoDelete(TRUE);

    loadingProject = FALSE;

    // Hide the 2nd column. It contains the report ID that is of no interest
    // to the user.
    browser->setColumnWidthMode(1, QListView::Manual);
    browser->hideColumn(1);
}

QListViewItem*
ReportManager::getFirstInteractiveReportItem() const
{
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri)
        if (strncmp((*mri)->getProjectReport()->getType(), "Qt", 2) == 0)
            return (*mri)->getBrowserEntry();

    return 0;
}

void
ReportManager::updateReportList(QPtrListIterator<Report> rli)
{
    reports.clear();
    for ( ; *rli; ++rli)
        reports.append(new ManagedReportInfo(this, *rli));

    updateReportBrowser();
}

void
ReportManager::updateReportBrowser()
{
    browser->clear();

    qtReports = new KListViewItem(browser, i18n("Interactive Reports"));
    htmlReports = new KListViewItem(browser, i18n("HTML Reports"));
    csvReports = new KListViewItem(browser, i18n("CSV Reports"));
    xmlReports = new KListViewItem(browser, i18n("XML Reports"));
    exportReports = new KListViewItem(browser, i18n("Export Reports"));

    int i = 0;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri, ++i)
    {
        Report* r = (*mri)->getProjectReport();
        KListViewItem* parent = 0;
        if (strncmp(r->getType(), "Qt", 2) == 0)
            parent = qtReports;
        else if (strncmp(r->getType(), "HTML", 4) == 0)
            parent = htmlReports;
        else if (strncmp(r->getType(), "CSV", 3) == 0)
            parent = csvReports;
        else if (strncmp(r->getType(), "XML", 3) == 0)
            parent = xmlReports;
        else if (strncmp(r->getType(), "Export", 6) == 0)
            parent = exportReports;
        else
            kdError() << "ReportManager::updateReportBrowser(): "
                << "Unsupported report type " << r->getType() << endl;

        KListViewItem* item = new KListViewItem
            (parent, r->getFileName(), QString::number(i),
             r->getDefinitionFile(), QString::number(r->getDefinitionLine()));

        // Save the pointer to the list view item for future references.
        (*mri)->setBrowserEntry(item);
    }
    qtReports->setOpen(TRUE);
}

bool
ReportManager::showReport(QListViewItem* lvi)
{
    ManagedReportInfo* mr = 0;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri)
        if ((*mri)->getBrowserEntry() == lvi)
            mr = *mri;

    if (!mr)
        return TRUE;

    TjReport* tjr;
    if ((tjr = mr->getReport()) == 0)
    {
        if (strcmp(mr->getProjectReport()->getType(), "QtTaskReport") == 0)
            tjr = new TjTaskReport(reportStack, mr->getProjectReport());
        else if (strcmp(mr->getProjectReport()->getType(),
                        "QtResourceReport") == 0)
            tjr = new TjResourceReport(reportStack, mr->getProjectReport());
        else if (strncmp(mr->getProjectReport()->getType(), "HTML", 4) == 0)
        {
            HTMLReport* htmlReport =
                dynamic_cast<HTMLReport*>(mr->getProjectReport());
            if (!htmlReport->generate())
                return FALSE;
            // show the HTML file in web browser
            KURL reportUrl =
                KURL::fromPathOrURL(mr->getProjectReport()->
                                    getDefinitionFile());
            reportUrl.setFileName(mr->getProjectReport()->getFileName());

            changeStatusBar(i18n("Displaying HTML report: '%1'")
                            .arg(mr->getProjectReport()->getFileName()));
            KRun::runURL(reportUrl, "text/html");
            return TRUE;
        }
        else
        {
            kdDebug() << "Report type " << mr->getProjectReport()->getType()
                << " not yet supported" << endl;
            return FALSE;
        }

        connect(tjr, SIGNAL(signalChangeStatusBar(const QString&)),
                this, SLOT(changeStatusBar(const QString&)));
        connect(tjr, SIGNAL(signalEditCoreAttributes(CoreAttributes*)),
                this, SLOT(editCoreAttributes(CoreAttributes*)));

        reportStack->addWidget(tjr);
        mr->setReport(tjr);

        if (!tjr->generateReport())
            return FALSE;
    }
    reportStack->raiseWidget(tjr);

    return TRUE;
}

ManagedReportInfo*
ReportManager::getCurrentReport() const
{
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri)
        if ((*mri)->getBrowserEntry() == browser->currentItem())
            return *mri;

    return 0;
}

void
ReportManager::closeCurrentReport()
{
    ManagedReportInfo* mri;
    if ((mri = getCurrentReport()) != 0)
    {
        reportStack->removeWidget(mri->getReport());
        reportStack->raiseWidget(0);
        reports.removeRef(mri);
        updateReportBrowser();
    }
}

void
ReportManager::changeStatusBar(const QString& text)
{
    emit signalChangeStatusBar(text);
}

void
ReportManager::editCoreAttributes(CoreAttributes* ca)
{
    emit signalEditCoreAttributes(ca);
}

void
ReportManager::clear()
{
    reports.clear();
}

void
ReportManager::zoomIn()
{
    if (loadingProject)
        return;

    if (getCurrentReport())
        getCurrentReport()->getReport()->zoomIn();
}

void
ReportManager::zoomOut()
{
    if (loadingProject)
        return;

    if (getCurrentReport())
        getCurrentReport()->getReport()->zoomOut();
}

void
ReportManager::setFocusToReport() const
{
    if (loadingProject)
        return;

    if (getCurrentReport() && getCurrentReport()->getReport())
        getCurrentReport()->getReport()->setFocus();
}

void
ReportManager::setLoadingProject(bool lp)
{
    loadingProject = lp;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri)
        (*mri)->setLoadingProject(lp);
}

#include "ReportManager.moc"
