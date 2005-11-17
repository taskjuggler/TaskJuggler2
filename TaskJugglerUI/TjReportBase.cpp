/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjReport.h 1181 2005-10-24 08:29:54Z cs $
 */

#include "TjReportBase.h"

TjReportBase::TjReportBase(QWidget* p, Report* const rDef, const QString& n) :
    QWidget(p, n), reportDef(rDef)
{
}

void
TjReportBase::print()
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
