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

#include "ManagedReportInfo.h"

#include "klocale.h"

#include "TjReport.h"

ManagedReportInfo::ManagedReportInfo(ReportManager* rm, Report* r) :
    manager(rm), projectReport(r)
{
    report = 0;
    browserEntry = 0;
    loadingProject = false;
}

ManagedReportInfo::~ManagedReportInfo()
{
    delete report;
}

const QString&
ManagedReportInfo::getName() const
{
    if (projectReport)
        return projectReport->getFileName();
    else
        return QString::null;
}

void
ManagedReportInfo::setLoadingProject(bool lp)
{
    loadingProject = lp;
    if (report)
        report->setLoadingProject(lp);
}

void
ManagedReportInfo::print()
{
    report->print();
}

