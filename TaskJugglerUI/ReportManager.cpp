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

#include "ReportManager.h"

#include <assert.h>

#include <qwidgetstack.h>
#include <qstring.h>
#include <qpopupmenu.h>

#include <kmainwindow.h>
#include <kaction.h>
#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kurl.h>
#include <krun.h>
#include <klistviewsearchline.h>
#include <kmessagebox.h>

#include "Project.h"
#include "Report.h"
#include "HTMLReport.h"
#include "ICalReport.h"
#include "CSVReport.h"
#include "XMLReport.h"
#include "ExportReport.h"

#include "ManagedReportInfo.h"
#include "TjReportBase.h"
#include "TjSummaryReport.h"
#include "TjHTMLReport.h"
#include "TjTaskReport.h"
#include "TjResourceReport.h"
#include "MainWidget.h"

ReportManager::ReportManager(KMainWindow* m, QWidgetStack* v,
                             KListView* b, KListViewSearchLine* s) :
    mainWindow(m), reportStack(v), browser(b), searchLine(s)
{
    loadingProject = FALSE;

    /* Hide the 2nd column. It contains the report ID that is of no interest
     * to the user. */
    browser->setColumnWidthMode(1, QListView::Manual);
    browser->hideColumn(1);

    // Setup zoom toolbar actions.
    zoomSelector = new KSelectAction
        (i18n("Zoom"), "viewmag", 0, this, SLOT(zoomTo()),
         m->actionCollection(), "zoom_to");
    zoomSelector->setEditable(false);
    zoomSelector->setComboWidth(170);
#if KDE_IS_VERSION(3,4,89)
    zoomSelector->setMaxComboViewCount(15);
#endif

}

ReportManager::~ReportManager()
{
    clear();
}

QListViewItem*
ReportManager::getFirstInteractiveReportItem() const
{
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
    {
        // Watch out for the summary report.
        if (!(*mri)->getProjectReport())
            continue;
        if (strncmp((*mri)->getProjectReport()->getType(), "Qt", 2) == 0)
            return (*mri)->getBrowserEntry();
    }

    return 0;
}

void
ReportManager::updateReportList(const Project* pr)
{
    clear();
    project = pr;

    QPtrListIterator<Report> rli = project->getReportListIterator();
    for ( ; *rli; ++rli)
        reports.push_front(new ManagedReportInfo(this, *rli));
    /* Add the summary report. There is no report definition in the project
     * for the summary report, so we pass a 0 pointer. */
    reports.push_front(new ManagedReportInfo(this, 0));

    updateReportBrowser();
}

void
ReportManager::updateReportBrowser()
{
    QStringList openReports;
    for (QListViewItemIterator lvi(browser); *lvi; ++lvi)
        if ((*lvi)->firstChild())
            if ((*lvi)->isOpen())
                openReports.append((*lvi)->text(0));
    QString currentReport;
    if (browser->currentItem() && browser->currentItem()->firstChild() == 0)
        currentReport = browser->currentItem()->text(0);

    browser->clear();

    qtReports = new KListViewItem(browser, i18n("Interactive Reports"));
    qtReports->setPixmap(0, KGlobal::iconLoader()->
                         loadIcon("tj_interactive_reports", KIcon::Small));
    qtReports->setOpen(openReports.isEmpty() ||
                       openReports.find(qtReports->text(0)) !=
                       openReports.end());

    htmlReports = new KListViewItem(browser, i18n("HTML Reports"));
    htmlReports->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_html_reports", KIcon::Small));
    htmlReports->setOpen(openReports.find(htmlReports->text(0)) !=
                         openReports.end());

    csvReports = new KListViewItem(browser, i18n("CSV Reports"));
    csvReports->setPixmap(0, KGlobal::iconLoader()->
                          loadIcon("tj_csv_reports", KIcon::Small));
    csvReports->setOpen(openReports.find(csvReports->text(0)) !=
                        openReports.end());

    xmlReports = new KListViewItem(browser, i18n("XML Reports"));
    xmlReports->setPixmap(0, KGlobal::iconLoader()->
                          loadIcon("tj_xml_reports", KIcon::Small));
    xmlReports->setOpen(openReports.find(xmlReports->text(0)) !=
                         openReports.end());

    icalReports = new KListViewItem(browser, i18n("iCalendars"));
    icalReports->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_ical_reports", KIcon::Small));
    icalReports->setOpen(openReports.find(icalReports->text(0)) !=
                         openReports.end());

    exportReports = new KListViewItem(browser, i18n("Export Reports"));
    exportReports->setPixmap(0, KGlobal::iconLoader()->
                             loadIcon("tj_export_reports", KIcon::Small));
    exportReports->setOpen(openReports.find(exportReports->text(0)) !=
                           openReports.end());

    int i = 0;
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
    {
        Report* r = (*mri)->getProjectReport();

        // The summary report has no report definition.
        if (!r)
            continue;

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
        else if (strncmp(subType, "Report", strlen("Report")) == 0)
            subTypeIcon =
                KGlobal::iconLoader()->loadIcon("tj_file_tji",
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

        // Restore current report.
        if (item->text(0) == currentReport)
            browser->setCurrentItem(item);

        // Save the pointer to the list view item for future references.
        (*mri)->setBrowserEntry(item);
    }

    // Make sure that we have a current report. If the current report is a
    // report folder, then select the first interactive report if it exists.
    if (browser->currentItem() == 0 ||
        browser->currentItem()->firstChild() != 0)
        if (qtReports->firstChild() != 0)
            browser->setCurrentItem(qtReports->firstChild());

    searchLine->updateSearch();
}

bool
ReportManager::generateReport(QListViewItem* lvi)
{
    ManagedReportInfo* mr = 0;
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
        if ((*mri)->getBrowserEntry() == lvi)
            mr = *mri;

    if (!mr)
        return TRUE;

    bool retVal = true;
    if (strncmp(mr->getProjectReport()->getType(), "Qt", 2) == 0)
    {
        // Nothing to do
        return true;
    }
    else if (strncmp(mr->getProjectReport()->getType(), "CSV", 3) == 0)
    {
        CSVReport* csvReport =
            dynamic_cast<CSVReport*>(mr->getProjectReport());
        retVal = csvReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "Export", 6) == 0)
    {
        ExportReport* exportReport =
            dynamic_cast<ExportReport*>(mr->getProjectReport());
        retVal = exportReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "HTML", 4) == 0)
    {
        HTMLReport* htmlReport =
            dynamic_cast<HTMLReport*>(mr->getProjectReport());
        retVal = htmlReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "ICal", 4) == 0)
    {
        ICalReport* icalReport =
            dynamic_cast<ICalReport*>(mr->getProjectReport());
        retVal = icalReport->generate();
    }
    else if (strncmp(mr->getProjectReport()->getType(), "XML", 3) == 0)
    {
        XMLReport* xmlReport =
            dynamic_cast<XMLReport*>(mr->getProjectReport());
        retVal = xmlReport->generate();
    }
    else
    {
        kdDebug() << "Report type " << mr->getProjectReport()->getType()
            << " not yet supported" << endl;
        return FALSE;
    }

    if (retVal)
        KMessageBox::information(0, i18n("The report '%1' has been generated")
                                 .arg(mr->getProjectReport()->getFileName()),
                                 QString::null, "ConfirmReportGeneration");
    else
        KMessageBox::error(0, i18n("An error occured while generating the "
                                   "report '%1'")
                           .arg(mr->getProjectReport()->getFileName()),
                           i18n("Report Generation failed"));

    return retVal;
}

bool
ReportManager::showReport(QListViewItem* lvi, bool& showReportTab)
{
    ManagedReportInfo* mr = 0;
    showReportTab = true;
    if (!lvi)
    {
        /* If the lvi is null then we try to show the current report or the
         * first interactive report. */
        if (browser->currentItem())
            lvi = browser->currentItem();
        else if (qtReports->firstChild())
            lvi = (qtReports->firstChild());
    }
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
        if ((*mri)->getBrowserEntry() == lvi)
            mr = *mri;

    if (!mr)
    {
        /* In case there is no corresponding list view entry we show the
         * summary report. The summary report has no report definition, so it
         * can be identified by a 0 pointer. */
        for (std::list<ManagedReportInfo*>::const_iterator
             mri = reports.begin();
             mri != reports.end(); ++mri)
            if ((*mri)->getProjectReport() == 0)
                mr = *mri;
    }

    TjReportBase* tjr = mr->getReport();
    if (tjr == 0)
    {
        if (mr->getProjectReport() == 0)
            tjr = new TjSummaryReport(reportStack, this, project);
        else if (strcmp(mr->getProjectReport()->getType(), "QtTaskReport") == 0)
            tjr = new TjTaskReport(reportStack, this, mr->getProjectReport());
        else if (strcmp(mr->getProjectReport()->getType(),
                        "QtResourceReport") == 0)
            tjr = new TjResourceReport(reportStack, this,
                                       mr->getProjectReport());
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
            tjr = new TjHTMLReport(reportStack, this, mr->getProjectReport());
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
        else if (strncmp(mr->getProjectReport()->getType(), "Export", 6) == 0)
        {
            // Generate the report file
            ExportReport* exportReport =
                dynamic_cast<ExportReport*>(mr->getProjectReport());
            if (!exportReport->generate())
                return false;

            // Get the full file name as URL and show it in the editor
            KURL reportUrl =
                KURL::fromPathOrURL(mr->getProjectReport()->getFullFileName());
            if (reportUrl.url().right(4) == ".tjp")
            {
                changeStatusBar(i18n("Starting new TaskJuggler for '%1'")
                    .arg(mr->getProjectReport()->getFileName()));
                KRun::runURL(reportUrl, "application/x-tjp");
            }
            else
            {
                emit signalEditFile(reportUrl);
                showReportTab = false;
            }
            return true;
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
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
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
        {
            bool dummy;
            errors = !showReport(lvi, dummy);
            break;
        }
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
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
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

        // Find the mri in the reports list, delete and remove it.
        for (std::list<ManagedReportInfo*>::iterator mrit =
             reports.begin(); mrit != reports.end(); ++mrit)
            if (*mrit == mri)
            {
                delete *mrit;
                reports.erase(mrit);
                mri = 0;
                break;
            }

        assert(mri == 0);

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
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
        delete *mri;

    reports.clear();
}

void
ReportManager::print()
{
    if (getCurrentReport())
        getCurrentReport()->getReport()->print();
}

void
ReportManager::zoomTo()
{
    if (loadingProject)
        return;

    if (getCurrentReport())
        getCurrentReport()->getReport()->zoomTo(zoomSelector->currentText());
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
ReportManager::updateZoomSelector(const QStringList& items,
                                  unsigned int current)
{
    zoomSelector->setItems(items);
    zoomSelector->setCurrentItem(current);
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
ReportManager::enableReportActions(bool enable)
{
    mainWindow->action("zoom_in")->setEnabled(enable);
    mainWindow->action("zoom_out")->setEnabled(enable);
}

void
ReportManager::setLoadingProject(bool lp)
{
    loadingProject = lp;
    for (std::list<ManagedReportInfo*>::const_iterator mri = reports.begin();
         mri != reports.end(); ++mri)
        (*mri)->setLoadingProject(lp);
}

#include "ReportManager.moc"
