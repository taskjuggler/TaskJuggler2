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
#include <qpopupmenu.h>

#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kurl.h>
#include <krun.h>
#include <kprinter.h>

#include "Report.h"
#include "HTMLReport.h"
#include "ICalReport.h"
#include "CSVReport.h"
#include "XMLReport.h"
#include "ExportReport.h"

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

    printer = 0;
}

ReportManager::~ReportManager()
{
    clear();
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
    qtReports->setPixmap(0, KGlobal::iconLoader()->
                         loadIcon("tj_interactive_reports", KIcon::Small));
    htmlReports = new KListViewItem(browser, i18n("HTML Reports"));
    htmlReports->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_html_reports", KIcon::Small));
    csvReports = new KListViewItem(browser, i18n("CSV Reports"));
    csvReports->setPixmap(0, KGlobal::iconLoader()->
                          loadIcon("tj_csv_reports", KIcon::Small));
    xmlReports = new KListViewItem(browser, i18n("XML Reports"));
    xmlReports->setPixmap(0, KGlobal::iconLoader()->
                          loadIcon("tj_xml_reports", KIcon::Small));
    icalReports = new KListViewItem(browser, i18n("iCalendars"));
    icalReports->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_ical_reports", KIcon::Small));
    exportReports = new KListViewItem(browser, i18n("Export Reports"));
    exportReports->setPixmap(0, KGlobal::iconLoader()->
                             loadIcon("tj_export_reports", KIcon::Small));

    int i = 0;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri, ++i)
    {
        Report* r = (*mri)->getProjectReport();
        KListViewItem* parent = 0;
        int prefix = 0;
        if (strncmp(r->getType(), "Qt", 2) == 0)
        {
            prefix = 2;
            parent = qtReports;
        }
        else if (strncmp(r->getType(), "HTML", 4) == 0)
        {
            prefix = 4;
            parent = htmlReports;
        }
        else if (strncmp(r->getType(), "CSV", 3) == 0)
        {
            prefix = 3;
            parent = csvReports;
        }
        else if (strncmp(r->getType(), "XML", 3) == 0)
        {
            prefix = 3;
            parent = xmlReports;
        }
        else if (strncmp(r->getType(), "ICal", 4) == 0)
        {
            prefix = 4;
            parent = icalReports;
        }
        else if (strncmp(r->getType(), "Export", 6) == 0)
        {
            prefix = 6;
            parent = exportReports;
        }
        else
            kdError() << "ReportManager::updateReportBrowser(): "
                << "Unsupported report type " << r->getType() << endl;

        QPixmap subTypeIcon;
        const char* subType = r->getType() + prefix;
        if (strncmp(subType, "Task", strlen("Task")) == 0)
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_task_group",
                                                KIcon::Small);
        else if (strncmp(subType, "Resource", strlen("Resource")) == 0)
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_resource_group",
                                                KIcon::Small);
        else if (strncmp(subType, "Account", strlen("Account")) == 0)
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_account_group",
                                                KIcon::Small);
        else if (strncmp(subType, "WeeklyCalendar",
                         strlen("WeeklyCalendar")) == 0)
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_calendar_report",
                                                KIcon::Small);
        else if (strncmp(subType, "Status", strlen("Status")) == 0)
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_status_report",
                                                KIcon::Small);
        else
        {
            qDebug("Unknown type: %s", r->getType());
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_report",
                                                KIcon::Small);
        }

        KListViewItem* item = new KListViewItem
            (parent, r->getFileName(), QString::number(i),
             r->getDefinitionFile(), QString::number(r->getDefinitionLine()));
        item->setPixmap(0, subTypeIcon);

        // Save the pointer to the list view item for future references.
        (*mri)->setBrowserEntry(item);
    }
    qtReports->setOpen(TRUE);
}

bool
ReportManager::generateReport(QListViewItem* lvi)
{
    ManagedReportInfo* mr = 0;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri)
        if ((*mri)->getBrowserEntry() == lvi)
            mr = *mri;

    if (!mr)
        return TRUE;

    if (strncmp(mr->getProjectReport()->getType(), "Qt", 2) == 0)
        ; // Nothing to do
    else if (strncmp(mr->getProjectReport()->getType(), "CSV", 3) == 0)
    {
        CSVReport* csvReport =
            dynamic_cast<CSVReport*>(mr->getProjectReport());
        return csvReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "Export", 6) == 0)
    {
        ExportReport* exportReport =
            dynamic_cast<ExportReport*>(mr->getProjectReport());
        return exportReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "HTML", 4) == 0)
    {
        HTMLReport* htmlReport =
            dynamic_cast<HTMLReport*>(mr->getProjectReport());
        return htmlReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "ICal", 4) == 0)
    {
        ICalReport* icalReport =
            dynamic_cast<ICalReport*>(mr->getProjectReport());
        return icalReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "XML", 3) == 0)
    {
        XMLReport* xmlReport =
            dynamic_cast<XMLReport*>(mr->getProjectReport());
        return xmlReport->generate();
    }
    else
    {
        kdDebug() << "Report type " << mr->getProjectReport()->getType()
            << " not yet supported" << endl;
        return FALSE;
    }

    return TRUE;
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
        else if (strncmp(mr->getProjectReport()->getType(), "CSV", 3) == 0)
        {
            CSVReport* csvReport =
                dynamic_cast<CSVReport*>(mr->getProjectReport());
            if (!csvReport->generate())
                return FALSE;
            // show the CSV file in preferred CSV handler
            KURL reportUrl =
                KURL::fromPathOrURL(mr->getProjectReport()->
                                    getDefinitionFile());
            reportUrl.setFileName(mr->getProjectReport()->getFileName());

            changeStatusBar(i18n("Displaying CSV report: '%1'")
                            .arg(mr->getProjectReport()->getFileName()));
            KRun::runURL(reportUrl, "text/x-csv");
            return TRUE;
        }
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
        else if (strncmp(mr->getProjectReport()->getType(), "ICal", 4) == 0)
        {
            ICalReport* icalReport =
                dynamic_cast<ICalReport*>(mr->getProjectReport());
            if (!icalReport->generate())
                return FALSE;
            // show the TODO list in Korganizer
            KURL reportUrl =
                KURL::fromPathOrURL(mr->getProjectReport()->
                                    getDefinitionFile());
            reportUrl.setFileName(mr->getProjectReport()->getFileName());

            changeStatusBar(i18n("Displaying iCalendar: '%1'")
                            .arg(mr->getProjectReport()->getFileName()));
            KRun::runURL(reportUrl, "text/calendar");
            return TRUE;
        }
        else
        {
            kdDebug() << "Report type " << mr->getProjectReport()->getType()
                << " not yet supported" << endl;
            return FALSE;
        }

        if (tjr)
        {
            connect(tjr, SIGNAL(signalChangeStatusBar(const QString&)),
                    this, SLOT(changeStatusBar(const QString&)));
            connect(tjr, SIGNAL(signalEditCoreAttributes(CoreAttributes*)),
                    this, SLOT(editCoreAttributes(CoreAttributes*)));

            if (!tjr->generateReport())
            {
                delete tjr;
                return FALSE;
            }

            reportStack->addWidget(tjr);
            mr->setReport(tjr);
        }
    }
    if (tjr)
        reportStack->raiseWidget(tjr);

    return TRUE;
}

void
ReportManager::showRMBMenu(QListViewItem* lvi, const QPoint& pos, int,
                           bool& errors, bool& showReportTab)
{
    ManagedReportInfo* mr = 0;
    for (QPtrListIterator<ManagedReportInfo> mri(reports); *mri; ++mri)
        if ((*mri)->getBrowserEntry() == lvi)
            mr = *mri;

    if (!mr)
        return;

    // Generate a context popup menu.
    QPopupMenu menu;
    menu.insertItem(i18n("&Show Report"), 1);
    menu.insertItem(i18n("&Generate Report"), 2);
    menu.insertItem(i18n("&Edit Report Definition"), 3);

    // The XML reports cannot be be viewed, so we disable the entry.
    if (strncmp(mr->getProjectReport()->getType(), "XML", 3) == 0)
        menu.setItemEnabled(1, FALSE);

    // The interactive reports can not be generated, so we disable the entry.
    if (strncmp(mr->getProjectReport()->getType(), "Qt", 2) == 0)
        menu.setItemEnabled(2, FALSE);

    switch (menu.exec(pos))
    {
        case 1:
            errors = !showReport(lvi);
            break;
        case 2:
            errors = !generateReport(lvi);
            showReportTab = FALSE;
            break;
        case 3:
            editReport(mr->getProjectReport());
            showReportTab = FALSE;
            break;
        default:
            break;
    }
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
ReportManager::editReport(const Report* report)
{
    emit signalEditReport(report);
}

void
ReportManager::clear()
{
    reports.clear();
}

void
ReportManager::print()
{
    // Looks like I have to create a KPrinter for each print job. Otherwise it
    // messes up the paper size.
    printer = new KPrinter(this);

    if (getCurrentReport())
        getCurrentReport()->getReport()->print(printer);

    delete printer;
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
