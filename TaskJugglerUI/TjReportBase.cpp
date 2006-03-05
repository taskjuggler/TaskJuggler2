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

#include "TjReportBase.h"

TjReportBase::TjReportBase(QWidget* p, ReportManager* m, Report* const rDef,
                           const QString& n) :
    QWidget(p, n), manager(m), reportDef(rDef)
{
}

void
TjReportBase::print()
{
}

void
TjReportBase::zoomTo(const QString&)
{
}

void
TjReportBase::zoomIn()
{
}

void
TjReportBase::zoomOut()
{
}

void
TjReportBase::show()
{
}

void
TjReportBase::hide()
{
}

#include "TjReportBase.moc"
