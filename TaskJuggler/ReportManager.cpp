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

#include "Report.h"
#include "ManagedReportInfo.h"
#include "TjReport.h"

ReportManager::ReportManager(QWidgetStack* v, KListView* b) :
    viewStack(v), browser(b)
{
    reports.setAutoDelete(TRUE);

    // Hide the 2nd column. It contains the report ID that is of no interest
    // to the user.
    browser->setColumnWidthMode(1, QListView::Manual);
    browser->hideColumn(1);
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

    int i = 0;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri, ++i)
    {
        Report* r = (*mri)->getProjectReport();
        KListViewItem* parent;
        if (strncmp(r->getType(), "Qt", 2) == 0)
            parent = qtReports;
        else if (strncmp(r->getType(), "HTML", 4) == 0)
            parent = htmlReports;
        else if (strncmp(r->getType(), "CSV", 3) == 0)
            parent = csvReports;
        else if (strncmp(r->getType(), "XML", 3) == 0)
            parent = xmlReports;
        else
            kdError() << "ReportManager::updateReportBrowser(): "
                << "Unsupported report type " << r->getType() << endl;

        KListViewItem* item = new KListViewItem
            (parent, r->getFileName(), QString().sprintf("%d", i),
             r->getDefinitionFile(),
             QString().sprintf("%d", r->getDefinitionLine()));

        // Save the pointer to the list view item for future references.
        (*mri)->setBrowserEntry(item);
    }
    qtReports->setOpen(TRUE);
}

void
ReportManager::showReport(KListViewItem*)
{
}

ManagedReportInfo*
ReportManager::getCurrentReport() const
{
    // TODO: Add real code!
    return 0;
}

void
ReportManager::closeCurrentReport()
{
    ManagedReportInfo* mri;
    if ((mri = getCurrentReport()) != 0)
    {
        viewStack->removeWidget(mri->getReport());
        viewStack->raiseWidget(0);
        reports.removeRef(mri);
        updateReportBrowser();
    }
}

void
ReportManager::clear()
{
    reports.clear();
}

void
ReportManager::setFocusToReport() const
{
    if (getCurrentReport())
        getCurrentReport()->getReport()->setFocus();
}

