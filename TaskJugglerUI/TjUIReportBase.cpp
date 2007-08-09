/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjUIReportBase.cpp 1275 2006-03-05 19:16:55Z cs $
 */

#include "TjUIReportBase.h"

TjUIReportBase::TjUIReportBase(QWidget* p, ReportManager* m, Report* rDef,
                           const QString& n) :
    QWidget(p, n), ReportElementBase(rDef), manager(m)
{
}

void
TjUIReportBase::print()
{
}

void
TjUIReportBase::zoomTo(const QString&)
{
}

void
TjUIReportBase::zoomIn()
{
}

void
TjUIReportBase::zoomOut()
{
}

void
TjUIReportBase::show()
{
}

void
TjUIReportBase::hide()
{
}

#include "TjUIReportBase.moc"
